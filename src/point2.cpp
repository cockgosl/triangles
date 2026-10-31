#include "point2.hpp"

#include <cmath>

namespace triangles {

Point2::Point2(const double x,
               const double y)
      : x_(x), y_(y) {}

std::optional<Point2> Point2::Create(const double x,
                                     const double y) {
    if (!std::isfinite(x) ||
        !std::isfinite(y))
        return std::nullopt;

    return Point2(x, y);
}

std::optional<Point2> Point2::Create(const Vector3& vector) {
    const double x_coord = vector.x;
    const double y_coord = vector.y;

    if (!std::isfinite(x_coord) ||
        !std::isfinite(y_coord))
        return std::nullopt;

    return Point2::Create(x_coord, y_coord);
}

Point2::~Point2() = default;

} // namespace triangles
