#pragma once

#include "vector3.hpp"

#include <optional>

namespace triangles {

class Plane3;

class Line3 {
private:
    Vector3 direction_;
    Vector3 point_;

    Line3(const Vector3& direction,
          const Vector3& point);
    explicit Line3(const Vector3& direction); // ctor via default point - beginning of coordinates
public:
    static std::optional<Line3> Create(const Vector3& direction,
                                       const Vector3& point);
    static std::optional<Line3> Create(const Plane3& plane1,
                                       const Plane3& plane2);
    static std::optional<Line3> Create(const Vector3& direction);

    ~Line3();

    const Vector3& get_direction() const { return direction_; }
    const Vector3& get_point() const { return point_; }

    bool operator==(const Line3& line) const;
};

} // namespace triangles
