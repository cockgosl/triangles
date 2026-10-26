#ifndef GRAPHICS_HPP_
#define GRAPHICS_HPP_

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <optional>
#include <string_view>

namespace graphics {

class ShaderProgram {
  private:
    GLuint program_id_ = 0;
    explicit ShaderProgram(GLuint program_id) noexcept;

  public:
    static std::optional<ShaderProgram>
    Create(std::string_view vertex_source, std::string_view fragment_source);

    ~ShaderProgram() noexcept;

    ShaderProgram(const ShaderProgram &) = delete;
    ShaderProgram &operator=(const ShaderProgram &) = delete;

    ShaderProgram(ShaderProgram &&rhs) noexcept;
    ShaderProgram &operator=(ShaderProgram &&rhs) noexcept;

    void SetMatrix4(const char *name, const glm::mat4 &matrix) const noexcept;

    void Use() const noexcept;
};

} // namespace graphics

#endif // GRAPHICS_HPP_
