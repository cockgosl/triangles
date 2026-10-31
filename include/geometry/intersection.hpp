#pragma once

#include "line3.hpp"
#include "plane3.hpp"
#include "triangle.hpp"

#include <optional>

namespace triangles {

std::optional<Line3> intersection(const Plane3& plane_1,
                                  const Plane3& plane_2);
std::optional<bool> intersection(const Triangle3& triangle_1,
                                 const Triangle3& triangle_2);

} // namespace triangles
