#include "shader_program.hpp"

#include <cstddef>
#include <iostream>
#include <limits>
#include <string>
#include <string_view>
#include <utility>

namespace graphics {
// next block is not a part of the API, so it's hidden in the empty namespace
namespace {

class ShaderHandle {
  private:
    void Reset() noexcept {
        if (id_ != 0) {
            glDeleteShader(id_);
            id_ = 0;
        }
    }

    GLuint id_ = 0;

  public:
    ShaderHandle() noexcept = default;

    explicit ShaderHandle(GLuint id) noexcept : id_(id) {}

    ~ShaderHandle() noexcept { Reset(); }

    ShaderHandle(const ShaderHandle &) = delete;
    ShaderHandle &operator=(const ShaderHandle &) = delete;

    ShaderHandle(ShaderHandle &&rhs) noexcept
        : id_(std::exchange(rhs.id_, GLuint{0})) {}

    ShaderHandle &operator=(ShaderHandle &&rhs) noexcept {
        if (this != &rhs) {
            Reset();
            id_ = std::exchange(rhs.id_, GLuint{0});
        }

        return *this;
    }

    explicit operator bool() const noexcept { return id_ != 0; }

    GLuint Get() const noexcept { return id_; }
};

std::string GetShaderLog(GLuint shader) {
    GLint log_length = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);

    if (log_length <= 1)
        return {};

    std::string log(static_cast<std::size_t>(log_length), '\0');

    GLsizei written = 0;
    glGetShaderInfoLog(shader, log_length, &written, log.data());
    log.resize(static_cast<std::size_t>(written));

    return log;
}

ShaderHandle CompileShader(GLenum type, std::string_view source) {
    if (source.empty()) {
        std::cerr << "Shader source is empty.\n";
        return {};
    }

    if (source.size() >
        static_cast<std::size_t>(std::numeric_limits<GLint>::max())) {
        std::cerr << "Shader source is too long.\n";
        return {};
    }

    const GLuint shader_id = glCreateShader(type);
    if (shader_id == 0) {
        std::cerr << "Failed to create shader.\n";
        return {};
    }

    ShaderHandle shader(shader_id);

    const GLchar *source_data = source.data();
    const GLint source_length = static_cast<GLint>(source.size());

    glShaderSource(shader.Get(),
                   1, // The amount of blocks, which are considered as output
                   &source_data, &source_length);

    glCompileShader(shader.Get());

    GLint compile_status = GL_FALSE;
    glGetShaderiv(shader.Get(), GL_COMPILE_STATUS, &compile_status);

    if (compile_status != GL_TRUE) {
        std::cerr << "Shader compilation failed:\n"
                  << GetShaderLog(shader.Get()) << '\n';
        return {};
    }

    return shader;
}

class ProgramHandle {
  private:
    void Reset() noexcept {
        if (id_ != 0) {
            glDeleteProgram(id_);
            id_ = 0;
        }
    }

    GLuint id_ = 0;

  public:
    ProgramHandle() noexcept = default;

    explicit ProgramHandle(GLuint id) noexcept : id_(id) {}

    ~ProgramHandle() noexcept { Reset(); }

    ProgramHandle(const ProgramHandle &) = delete;
    ProgramHandle &operator=(const ProgramHandle &) = delete;

    ProgramHandle(ProgramHandle &&rhs) noexcept
        : id_(std::exchange(rhs.id_, GLuint{0})) {}

    ProgramHandle &operator=(ProgramHandle &&rhs) noexcept {
        if (this != &rhs) {
            Reset();
            id_ = std::exchange(rhs.id_, GLuint{0});
        }

        return *this;
    }

    explicit operator bool() const noexcept { return id_ != 0; }

    GLuint Get() const noexcept { return id_; }

    GLuint Release() noexcept { return std::exchange(id_, GLuint{0}); }
};

std::string GetProgramLog(GLuint program) {
    GLint log_length = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);

    if (log_length <= 1) {
        return {};
    }

    std::string log(static_cast<std::size_t>(log_length), '\0');

    GLsizei written = 0;
    glGetProgramInfoLog(program, log_length, &written, log.data());
    log.resize(static_cast<std::size_t>(written));

    return log;
}

ProgramHandle LinkProgram(const ShaderHandle &vertex_shader,
                          const ShaderHandle &fragment_shader) {
    const GLuint program_id = glCreateProgram();

    if (program_id == 0) {
        std::cerr << "Failed to create shader program.\n";
        return {};
    }

    ProgramHandle program(program_id);

    glAttachShader(program.Get(), vertex_shader.Get());
    glAttachShader(program.Get(), fragment_shader.Get());

    glLinkProgram(program.Get());

    GLint link_status = GL_FALSE;
    glGetProgramiv(program.Get(), GL_LINK_STATUS, &link_status);

    if (link_status != GL_TRUE) {
        std::cerr << "Shader program linking failed:\n"
                  << GetProgramLog(program.Get()) << '\n';
        return {};
    }

    return program;
}

} // namespace

ShaderProgram::ShaderProgram(GLuint program_id) noexcept
    : program_id_(program_id) {}

std::optional<ShaderProgram>
ShaderProgram::Create(std::string_view vertex_source,
                      std::string_view fragment_source) {

    ShaderHandle vertex_shader = CompileShader(GL_VERTEX_SHADER, vertex_source);

    if (!vertex_shader)
        return std::nullopt;

    ShaderHandle fragment_shader =
        CompileShader(GL_FRAGMENT_SHADER, fragment_source);

    if (!fragment_shader)
        return std::nullopt;

    ProgramHandle program = LinkProgram(vertex_shader, fragment_shader);

    if (!program)
        return std::nullopt;

    return ShaderProgram(program.Release());
}

ShaderProgram::~ShaderProgram() noexcept {
    if (program_id_ != 0)
        glDeleteProgram(program_id_);
}

ShaderProgram::ShaderProgram(ShaderProgram &&rhs) noexcept
    : program_id_(std::exchange(rhs.program_id_, GLuint{0})) {}

ShaderProgram &ShaderProgram::operator=(ShaderProgram &&rhs) noexcept {
    if (this != &rhs) {
        if (program_id_ != 0)
            glDeleteProgram(program_id_);

        program_id_ = std::exchange(rhs.program_id_, GLuint{0});
    }

    return *this;
}

void ShaderProgram::SetInt(const std::string &name, int value) const noexcept {
    const GLint location = glGetUniformLocation(program_id_, name.c_str());

    if (location < 0) {
        return;
    }

    glUniform1i(location, value);
}

void ShaderProgram::SetFloat(const char *name, float value) const noexcept {
    if (name == nullptr) {
        return;
    }

    const GLint location = glGetUniformLocation(program_id_, name);

    if (location < 0) {
        return;
    }

    glUniform1f(location, value);
}

void ShaderProgram::SetMatrix4(const char *name,
                               const glm::mat4 &matrix) const noexcept {
    if (name == nullptr) {
        return;
    }

    const GLint location = glGetUniformLocation(program_id_, name);

    if (location < 0) {
        return;
    }

    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
}

void ShaderProgram::Use() const noexcept {
    if (program_id_ != 0)
        glUseProgram(program_id_);
}

} // namespace graphics
