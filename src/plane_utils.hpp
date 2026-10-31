#pragma once

#include "plane3.hpp"

namespace triangles::detail {

bool is_parallel(const Plane3& plane1, const Plane3& plane2);
bool is_coincide(const Plane3& plane1, const Plane3& plane2);

} // namespace triangles::detail
