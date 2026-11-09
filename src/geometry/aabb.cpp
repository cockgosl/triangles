#include "geometry/aabb.hpp"

#include <array>
#include <cmath>
#include <numeric>

namespace triangles {

AABB::AABB(const Vector3& min,
           const Vector3& max)
    : min_(min),
      max_(max) {}

std::optional<AABB> AABB::Create(const Vector3& min,
                                 const Vector3& max) {
    const std::array coordinates{min.x, min.y, min.z, max.x, max.y, max.z};

    for (const double value : coordinates)
        if (!std::isfinite(value))
            return std::nullopt;

    if (min.x > max.x || min.y > max.y || min.z > max.z)
        return std::nullopt;

    return AABB(min, max);
}

const Vector3& AABB::get_min() const noexcept {
    return min_;
}

const Vector3& AABB::get_max() const noexcept {
    return max_;
}

bool AABB::intersects(const AABB& other) const noexcept {
    return !(max_.x < other.min_.x || min_.x > other.max_.x ||
             max_.y < other.min_.y || min_.y > other.max_.y ||
             max_.z < other.min_.z || min_.z > other.max_.z);
}

bool AABB::contains(const AABB& other) const noexcept {
    return min_.x <= other.min_.x && min_.y <= other.min_.y &&
           min_.z <= other.min_.z && max_.x >= other.max_.x &&
           max_.y >= other.max_.y && max_.z >= other.max_.z;
}

Vector3 AABB::center() const noexcept {
    return Vector3{
        std::midpoint(min_.x, max_.x),
        std::midpoint(min_.y, max_.y),
        std::midpoint(min_.z, max_.z) };
}

Vector3 AABB::extent() const noexcept {
    return max_ - min_;
}

} // namespace triangles
