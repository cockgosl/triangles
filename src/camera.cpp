#include "camera.hpp"

#include <algorithm>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <limits>

namespace graphics {
void Camera::SetAspectRatio(float aspect_ratio) {
    if (!std::isfinite(aspect_ratio) || aspect_ratio <= 0.0f) {
        return;
    }

    aspect_ratio_ = aspect_ratio;
}
glm::vec3 Camera::GetFrontDirection() const {
    const float yaw = glm::radians(yaw_degrees_);
    const float pitch = glm::radians(pitch_degrees_);

    glm::vec3 front;

    front.x = std::cos(yaw) * std::cos(pitch);
    front.y = std::sin(pitch);
    front.z = std::sin(yaw) * std::cos(pitch);

    return glm::normalize(front);
}
glm::vec3 Camera::GetRightDirection() const {
    const glm::vec3 front = GetFrontDirection();

    return glm::normalize(glm::cross(front, world_up_));
}
void Camera::Rotate(float x_offset, float y_offset) {
    yaw_degrees_ += x_offset * config_.mouse_sensitivity;
    pitch_degrees_ += y_offset * config_.mouse_sensitivity;

    pitch_degrees_ = std::clamp(pitch_degrees_, -89.0f, 89.0f);
}

void Camera::Move(CameraMovement direction, float delta_time) {
    if (delta_time <= 0.0f) {
        return;
    }

    const float distance = config_.movement_speed * delta_time;

    switch (direction) {
    case CameraMovement::kForward:
        position_ += GetFrontDirection() * distance;
        break;

    case CameraMovement::kBackward:
        position_ -= GetFrontDirection() * distance;
        break;

    case CameraMovement::kLeft:
        position_ -= GetRightDirection() * distance;
        break;

    case CameraMovement::kRight:
        position_ += GetRightDirection() * distance;
        break;
    }
}
glm::mat4 Camera::GetViewMatrix() const {
    const glm::vec3 front = GetFrontDirection();

    return glm::lookAt(position_, position_ + front, world_up_);
}
glm::mat4 Camera::GetProjectionMatrix() const {
    return glm::perspective(glm::radians(config_.field_of_view_degrees),
                            aspect_ratio_, config_.near_plane,
                            config_.far_plane);
}
} // namespace graphics
