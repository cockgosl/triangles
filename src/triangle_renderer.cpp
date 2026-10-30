#include "triangle_renderer.hpp"

#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace graphics {
namespace {

constexpr std::size_t kVertexComponentCount = 6;
constexpr float kMinShadowNearPlane = 0.01f;

std::optional<glm::mat4> GetLightSpaceMatrix(const std::vector<float> &vertices,
                                             const ShadowConfig &config) {
    const glm::mat4 light_view = glm::lookAt(
        config.light_position, config.light_target, config.light_up);

    glm::vec3 min_bounds{std::numeric_limits<float>::max()};

    glm::vec3 max_bounds{std::numeric_limits<float>::lowest()};

    // Calculate scene bounds in light space.
    for (std::size_t index = 0; index < vertices.size();
         index += kVertexComponentCount) {

        const glm::vec4 light_position =
            light_view * glm::vec4{vertices[index], vertices[index + 1],
                                   vertices[index + 2], 1.0f};

        const glm::vec3 position{light_position};

        min_bounds = glm::min(min_bounds, position);
        max_bounds = glm::max(max_bounds, position);
    }

    const glm::vec2 center{(min_bounds.x + max_bounds.x) * 0.5f,
                           (min_bounds.y + max_bounds.y) * 0.5f};

    const float width = max_bounds.x - min_bounds.x;
    const float height = max_bounds.y - min_bounds.y;

    // Use a square projection to preserve shadow map texel density.
    const float half_extent =
        std::max(width, height) * 0.5f + config.projection_padding;

    const float near_plane = std::max(
        kMinShadowNearPlane, -max_bounds.z - config.projection_padding);

    const float far_plane = -min_bounds.z + config.projection_padding;

    if (far_plane <= near_plane) {
        return std::nullopt;
    }

    const glm::mat4 light_projection = glm::ortho(
        center.x - half_extent, center.x + half_extent, center.y - half_extent,
        center.y + half_extent, near_plane, far_plane);

    return light_projection * light_view;
}

std::optional<std::string> ReadTextFile(const std::string &path) {
    std::ifstream file(path);

    if (!file) {
        std::cerr << "Failed to open file: " << path << '\n';
        return std::nullopt;
    }

    std::string contents{std::istreambuf_iterator<char>{file},
                         std::istreambuf_iterator<char>{}};

    if (file.bad()) {
        std::cerr << "Failed to read file: " << path << '\n';
        return std::nullopt;
    }

    return contents;
}

} // namespace

TriangleRenderer::~TriangleRenderer() noexcept {
    if (vbo_ != 0) {
        glDeleteBuffers(1, &vbo_);
        vbo_ = 0;
    }

    if (vao_ != 0) {
        glDeleteVertexArrays(1, &vao_);
        vao_ = 0;
    }

    if (shadow_depth_texture_ != 0) {
        glDeleteTextures(1, &shadow_depth_texture_);
        shadow_depth_texture_ = 0;
    }

    if (shadow_framebuffer_ != 0) {
        glDeleteFramebuffers(1, &shadow_framebuffer_);
        shadow_framebuffer_ = 0;
    }
}

bool TriangleRenderer::InitializeShadowMap() {
    glGenFramebuffers(1, &shadow_framebuffer_);
    glGenTextures(1, &shadow_depth_texture_);

    if (shadow_framebuffer_ == 0 || shadow_depth_texture_ == 0) {
        std::cerr << "Failed to create shadow map resources.\n";
        return false;
    }

    glBindTexture(GL_TEXTURE_2D, shadow_depth_texture_);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24,
                 shadow_config_.shadow_map_size, shadow_config_.shadow_map_size,
                 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    const float border_color[] = {1.0f, 1.0f, 1.0f, 1.0f};

    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);

    glBindFramebuffer(GL_FRAMEBUFFER, shadow_framebuffer_);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                           shadow_depth_texture_, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Shadow framebuffer is incomplete: " << status << '\n';

        return false;
    }

    return true;
}

bool TriangleRenderer::Initialize(const std::vector<float> &vertices,
                                  const ShadowConfig &shadow_config) {
    auto vertex_source = ReadTextFile("shaders/triangle.vert.glsl");

    auto fragment_source = ReadTextFile("shaders/triangle.frag.glsl");

    auto depth_vertex_source = ReadTextFile("shaders/depth.vert.glsl");

    auto depth_fragment_source = ReadTextFile("shaders/depth.frag.glsl");

    if (!(vertex_source && fragment_source && depth_vertex_source &&
          depth_fragment_source)) {
        return false;
    }

    if (shadow_config_.projection_padding <= 0.0f ||
        shadow_config_.min_bias < 0.0f ||
        shadow_config_.slope_bias_scale < 0.0f ||
        shadow_config_.max_bias < shadow_config_.min_bias ||
        shadow_config_.strength < 0.0f || shadow_config_.strength > 1.0f) {
        std::cerr << "Invalid shadow configuration.\n";
        return false;
    }

    shadow_config_ = shadow_config;

    auto shader_program =
        ShaderProgram::Create(*vertex_source, *fragment_source);

    auto depth_shader_program =
        ShaderProgram::Create(*depth_vertex_source, *depth_fragment_source);

    if (!depth_shader_program) {
        std::cerr << "Failed to create depth shader program.\n";
        return false;
    }
    if (!shader_program) {
        std::cerr << "Failed to create shader program.\n";
        return false;
    }

    depth_shader_program_.emplace(std::move(*depth_shader_program));
    shader_program_.emplace(std::move(*shader_program));

    shader_program_->Use();

    const glm::vec3 light_direction = glm::normalize(
        shadow_config_.light_position - shadow_config_.light_target);

    shader_program_->SetVec3("lightDirection", light_direction);
    shader_program_->SetInt("shadowMap", 0);
    shader_program_->SetFloat("minShadowBias", shadow_config_.min_bias);
    shader_program_->SetFloat("slopeBiasScale",
                              shadow_config_.slope_bias_scale);
    shader_program_->SetFloat("maxShadowBias", shadow_config_.max_bias);
    shader_program_->SetFloat("shadowStrength", shadow_config_.strength);

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);

    if (vao_ == 0 || vbo_ == 0) {
        std::cerr << "Failed to create OpenGL buffers.\n";
        return false;
    }

    glBindVertexArray(vao_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(vertices.size() * sizeof(float)),
                 vertices.data(), GL_STATIC_DRAW);

    constexpr GLsizei kVertexStride = static_cast<GLsizei>(6 * sizeof(float));

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, kVertexStride, nullptr);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, kVertexStride,
                          reinterpret_cast<void *>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    vertex_count_ = static_cast<GLsizei>(vertices.size() / 6);

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);

    if (!InitializeShadowMap()) {
        return false;
    }
    const glm::mat4 model{1.0f};

    auto light_space_matrix = GetLightSpaceMatrix(vertices, shadow_config_);

    if (!light_space_matrix) {
        std::cerr << "Failed to calculate light space matrix.\n";
        return false;
    }

    light_space_matrix_ = *light_space_matrix;

    RenderShadowPass(model, light_space_matrix_);
    RenderShadowPass(model, light_space_matrix_);

    return true;
}

void TriangleRenderer::RenderShadowPass(
    const glm::mat4 &model,
    const glm::mat4 &light_space_matrix) const noexcept {
    GLint viewport[4];
    GLint previous_framebuffer = 0;

    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previous_framebuffer);

    glViewport(0, 0, shadow_config_.shadow_map_size,
               shadow_config_.shadow_map_size);

    glBindFramebuffer(GL_FRAMEBUFFER, shadow_framebuffer_);
    glClear(GL_DEPTH_BUFFER_BIT);

    depth_shader_program_->Use();

    depth_shader_program_->SetMatrix4("model", model);
    depth_shader_program_->SetMatrix4("lightSpaceMatrix", light_space_matrix);

    glBindVertexArray(vao_);

    glEnable(GL_POLYGON_OFFSET_FILL);

    glPolygonOffset(shadow_config_.polygon_offset_factor,
                    shadow_config_.polygon_offset_units);

    glDrawArrays(GL_TRIANGLES, 0, vertex_count_);

    glDisable(GL_POLYGON_OFFSET_FILL);

    glBindVertexArray(0);

    glBindFramebuffer(GL_FRAMEBUFFER,
                      static_cast<GLuint>(previous_framebuffer));

    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}

void TriangleRenderer::Render(const glm::mat4 &view,
                              const glm::mat4 &projection) const noexcept {
    if (!shader_program_ || !depth_shader_program_ || vao_ == 0 ||
        shadow_framebuffer_ == 0) {
        return;
    }

    const glm::mat4 model{1.0f};

    // Second pass: render the scene from the camera's point of view.
    shader_program_->Use();

    shader_program_->SetMatrix4("model", model);
    shader_program_->SetMatrix4("view", view);
    shader_program_->SetMatrix4("projection", projection);
    shader_program_->SetMatrix4("lightSpaceMatrix", light_space_matrix_);

    // Bind the shadow map to texture unit 0.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, shadow_depth_texture_);

    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, vertex_count_);
    glBindVertexArray(0);

    glBindTexture(GL_TEXTURE_2D, 0);
}

} // namespace graphics
