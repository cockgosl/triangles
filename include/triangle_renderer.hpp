#ifndef TRIANGLE_RENDERER_HPP_
#define TRIANGLE_RENDERER_HPP_

#include "shader_program.hpp"

#include <glad/gl.h>

#include <optional>
#include <vector>

namespace graphics {

class TriangleRenderer final {
private:
std::optional<ShaderProgram> shader_program_;

GLuint vao_ = 0;
GLuint vbo_ = 0;
GLsizei vertex_count_ = 0;

public:
    TriangleRenderer() = default;
    ~TriangleRenderer() noexcept;

    TriangleRenderer(const TriangleRenderer&) = delete;
    TriangleRenderer& operator=(const TriangleRenderer&) = delete;

    bool Initialize(const std::vector<float>& vertices);
    void Render() const noexcept;
};

}  // namespace graphics

#endif
