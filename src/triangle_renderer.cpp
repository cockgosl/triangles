#include "triangle_renderer.hpp"

#include <glad/gl.h>

#include <fstream>
#include <iostream>
#include <iterator>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace graphics {
namespace {

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
}

bool TriangleRenderer::Initialize(const std::vector<float> &vertices) {
    auto vertex_source = ReadTextFile("shaders/triangle.vert.glsl");

    auto fragment_source = ReadTextFile("shaders/triangle.frag.glsl");

    if (!vertex_source || !fragment_source)
        return false;

    auto shader_program =
        ShaderProgram::Create(*vertex_source, *fragment_source);

    if (!shader_program) {
        std::cerr << "Failed to create shader program.\n";
        return false;
    }

    shader_program_.emplace(std::move(*shader_program));

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

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          static_cast<GLsizei>(3 * sizeof(float)), nullptr);

    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    vertex_count_ = static_cast<GLsizei>(vertices.size() / 3);

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);

    return true;
}

void TriangleRenderer::Render(const glm::mat4 &view,
                              const glm::mat4 &projection) const noexcept {
    if (!shader_program_ || vao_ == 0) {
        return;
    }

    shader_program_->Use();

    const glm::mat4 model{1.0f};

    shader_program_->SetMatrix4("model", model);
    shader_program_->SetMatrix4("view", view);
    shader_program_->SetMatrix4("projection", projection);

    glBindVertexArray(vao_);

    glDrawArrays(GL_TRIANGLES, 0, vertex_count_);

    glBindVertexArray(0);
}

} // namespace graphics
