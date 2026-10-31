#ifndef TRIANGLE_RENDERER_HPP_
#define TRIANGLE_RENDERER_HPP_

#include "shader_program.hpp"

#include <glad/gl.h>
#include <glm/glm.hpp>

#include <optional>
#include <vector>

namespace graphics {

struct ShadowConfig {
    int shadow_map_size{2048};

    glm::vec3 light_position{2.5f, 3.0f, 2.0f};
    glm::vec3 light_target{0.0f, 0.0f, 0.0f};
    glm::vec3 light_up{0.0f, 1.0f, 0.0f};

    float projection_padding{0.1f};

    float min_bias{0.0005f};
    float slope_bias_scale{0.002f};
    float max_bias{0.01f};

    float strength{0.7f};

    float polygon_offset_factor{1.1f};
    float polygon_offset_units{4.0f};
};

class TriangleRenderer final {
  private:
    std::optional<ShaderProgram> shader_program_;
    std::optional<ShaderProgram> depth_shader_program_;

    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    GLsizei vertex_count_ = 0;

    ShadowConfig shadow_config_;
    glm::mat4 light_space_matrix_{1.0f};
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

    bool Initialize(const std::vector<float> &vertices,
                    const ShadowConfig &shadow_config = {});
    void Render(const glm::mat4 &view,
                const glm::mat4 &projection) const noexcept;
};

} // namespace graphics

#endif
