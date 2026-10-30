#include "triangle_renderer.hpp"

#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>

#include <fstream>
#include <iostream>
#include <iterator>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace graphics {
namespace {

glm::mat4 GetLightSpaceMatrix(const ShadowConfig &config) {
    const glm::mat4 light_view = glm::lookAt(
        config.light_position, config.light_target, config.light_up);

    const glm::mat4 light_projection = glm::ortho(
        -config.projection_half_width, config.projection_half_width,
        -config.projection_half_height, config.projection_half_height,
        config.near_plane, config.far_plane);

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
    shader_program_->SetFloat("shadowBias", shadow_config_.bias);
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

    light_space_matrix_ = GetLightSpaceMatrix(shadow_config_);

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
