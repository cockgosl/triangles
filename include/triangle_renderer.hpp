#ifndef TRIANGLE_RENDERER_HPP_
#define TRIANGLE_RENDERER_HPP_

#include "shader_program.hpp"

#include <glad/gl.h>
#include <glm/glm.hpp>

#include <optional>
#include <vector>

namespace graphics {

struct ShadowConfig {
    // Shadow map settings.
    int shadow_map_size{2048};

    // Directional light settings.
    glm::vec3 light_position{2.5f, 3.0f, 2.0f};
    glm::vec3 light_target{0.0f, 0.0f, 0.0f};
    glm::vec3 light_up{0.0f, 1.0f, 0.0f};

    // Orthographic projection settings.
    float projection_half_width{2.0f};
    float projection_half_height{2.0f};
    float near_plane{0.1f};
    float far_plane{10.0f};

    // Shadow calculation settings.
    float bias{0.005f};
    float strength{0.7f};
};

class TriangleRenderer final {
  private:
    std::optional<ShaderProgram> shader_program_;
    std::optional<ShaderProgram> depth_shader_program_;

    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    GLsizei vertex_count_ = 0;

    ShadowConfig shadow_config_;
    GLuint shadow_framebuffer_ = 0;
    GLuint shadow_depth_texture_ = 0;

    bool InitializeShadowMap();
    void RenderShadowPass(const glm::mat4 &model,
                          const glm::mat4 &light_space_matrix) const noexcept;

  public:
    TriangleRenderer() = default;
    ~TriangleRenderer() noexcept;

    TriangleRenderer(const TriangleRenderer &) = delete;
    TriangleRenderer &operator=(const TriangleRenderer &) = delete;

    bool Initialize(const std::vector<float> &vertices, const ShadowConfig& shadow_config = {});
    void Render(const glm::mat4 &view,
                const glm::mat4 &projection) const noexcept;
};

} // namespace graphics

#endif
