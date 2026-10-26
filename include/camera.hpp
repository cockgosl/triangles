#include <glm/glm.hpp>

namespace graphics {

struct CameraConfig {
    float field_of_view_degrees = 45.0f;

    float movement_speed = 2.5f;
    float mouse_sensitivity = 0.1f;

    float near_plane = 0.1f;
    float far_plane = 100.0f;
};

enum class CameraMovement { kForward, kBackward, kLeft, kRight };

class Camera {
private:
    glm::vec3 position_ = {0.0f, 0.0f, 3.0f};
    float aspect_ratio_ = 4.0f / 3.0f;

    const glm::vec3 world_up_ = {0.0f, 1.0f, 0.0f};

    float yaw_degrees_ = -90.0f;
    float pitch_degrees_ = 0.0f;

    CameraConfig config_{};

public:
    Camera(const glm::vec3 &position, float aspect_ratio,
           const CameraConfig &config)
        : position_(position), aspect_ratio_(aspect_ratio), config_(config) {}

    void SetAspectRatio(float aspect_ration); 

    void Move(CameraMovement direction, float delta_time);
    void Rotate(float x_offset, float y_offset);

    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix() const;

    glm::vec3 GetFrontDirection() const;
    glm::vec3 GetRightDirection() const;
};

} // namespace graphics
