#ifndef AABB_HPP_
#define AABB_HPP_

#include "vector3.hpp"

#include <optional>

namespace triangles {

class AABB {
private:
    Vector3 min_;
    Vector3 max_;

    AABB(const Vector3& min, const Vector3& max);

public:
    static std::optional<AABB> Create(
        const Vector3& min,
        const Vector3& max
    );

    const Vector3& get_min() const noexcept;
    const Vector3& get_max() const noexcept;

    bool intersects(const AABB& other) const noexcept;
    bool contains(const AABB& other) const noexcept;

    Vector3 center() const noexcept;
    Vector3 extent() const noexcept;
};

}  // namespace triangles

#endif //AABB_HPP_
