#include "geometry/intersection.hpp"
#include "geometry/robust_predicates.hpp"

#include <algorithm>
#include <array>
#include <optional>
#include <ranges>
#include <stdexcept>

namespace triangles {

namespace {

using robust::Coordinates2;
using robust::Coordinates3;
using robust::Sign;
using TriangleCoordinates = std::array<Coordinates3, 3>;
using ProjectedTriangle = std::array<Coordinates2, 3>;
using Projection = std::array<std::size_t, 2>;
using PlaneSides = std::array<Sign, 3>;

TriangleCoordinates coordinates(const Triangle3& triangle) {
    const auto point = [](const Vector3& vertex) -> Coordinates3 { return {vertex.x, vertex.y, vertex.z}; };
    return {point(triangle.a), point(triangle.b), point(triangle.c)};
}

Coordinates2 project(const Coordinates3& point,
                     const Projection& axes) {
    return {point[axes[0]], point[axes[1]]};
}

ProjectedTriangle project(const TriangleCoordinates& triangle,
                          const Projection& axes) {
    return {project(triangle[0], axes), project(triangle[1], axes), project(triangle[2], axes)};
}

// TODO - throw logic
ProjectedTriangle project_nondegenerate(const TriangleCoordinates& triangle,
                                        Projection& axes) {
    constexpr std::array<Projection, 3> projections{{{0, 1}, {1, 2}, {2, 0}}};
    for (const auto& candidate : projections) {
        const auto projected = project(triangle, candidate);
        if (robust::orient2d(projected[0], projected[1], projected[2]) != Sign::zero) {
            axes = candidate;
            return projected;
        }
    }
    throw std::logic_error("Expected a nondegenerate triangle.");
}

bool same_nonzero(Sign first,
                  Sign second) {
    return first != Sign::zero && first == second;
}

bool opposite(Sign first,
              Sign second) {
    return first != Sign::zero && second != Sign::zero && first != second;
}

bool consistent_sides(const PlaneSides& sides) {
    return std::ranges::all_of(sides, [](Sign side) { return side != Sign::negative; }) ||
           std::ranges::all_of(sides, [](Sign side) { return side != Sign::positive; });
}

bool point_inside(const Coordinates2& point,
                  const ProjectedTriangle& triangle) {
    return consistent_sides({robust::orient2d(triangle[0], triangle[1], point), robust::orient2d(triangle[1], triangle[2], point),
                             robust::orient2d(triangle[2], triangle[0], point)});
}

bool in_segment_bounds(const Coordinates2& point,
                       const Coordinates2& first,
                       const Coordinates2& second) {
    return point[0] >= std::min(first[0], second[0]) && point[0] <= std::max(first[0], second[0]) &&
           point[1] >= std::min(first[1], second[1]) && point[1] <= std::max(first[1], second[1]);
}

bool segments_intersect(const Coordinates2& a,
                        const Coordinates2& b,
                        const Coordinates2& c,
                        const Coordinates2& d) {
    const auto ab_c = robust::orient2d(a, b, c);
    const auto ab_d = robust::orient2d(a, b, d);
    const auto cd_a = robust::orient2d(c, d, a);
    const auto cd_b = robust::orient2d(c, d, b);

    if (ab_c == Sign::zero && in_segment_bounds(c, a, b))
        return true;
    if (ab_d == Sign::zero && in_segment_bounds(d, a, b))
        return true;
    if (cd_a == Sign::zero && in_segment_bounds(a, c, d))
        return true;
    if (cd_b == Sign::zero && in_segment_bounds(b, c, d))
        return true;
    return opposite(ab_c, ab_d) && opposite(cd_a, cd_b);
}

bool coplanar_segment_intersects(const Coordinates3& first,
                                 const Coordinates3& second,
                                 const TriangleCoordinates& triangle) {
    Projection axes{};
    const auto projected_triangle = project_nondegenerate(triangle, axes);
    const auto a = project(first, axes);
    const auto b = project(second, axes);
    if (point_inside(a, projected_triangle) ||
        point_inside(b, projected_triangle))
        return true;
    for (std::size_t edge = 0; edge < 3; ++edge)
        if (segments_intersect(a, b, projected_triangle[edge], projected_triangle[(edge + 1) % 3]))
            return true;
    return false;
}

bool segment_intersects(const Coordinates3& first,
                        const Coordinates3& second,
                        Sign first_side,
                        Sign second_side,
                        const TriangleCoordinates& triangle) {
    if (same_nonzero(first_side, second_side))
        return false;
    if (first_side == Sign::zero && second_side == Sign::zero)
        return coplanar_segment_intersects(first, second, triangle);

    // The plane-side test places the line's hit within the closed segment
    return consistent_sides({robust::orient3d(first, second, triangle[0], triangle[1]),
                             robust::orient3d(first, second, triangle[1], triangle[2]),
                             robust::orient3d(first, second, triangle[2], triangle[0])});
}

PlaneSides plane_sides(const TriangleCoordinates& plane,
                       const TriangleCoordinates& triangle) {
    return {robust::orient3d(plane[0], plane[1], plane[2], triangle[0]),
            robust::orient3d(plane[0], plane[1], plane[2], triangle[1]),
            robust::orient3d(plane[0], plane[1], plane[2], triangle[2])};
}

bool strictly_one_side(const PlaneSides& sides) {
    return same_nonzero(sides[0], sides[1]) && same_nonzero(sides[0], sides[2]);
}

bool triangles_intersect(const TriangleCoordinates& first,
                         const TriangleCoordinates& second) {
    const auto first_sides = plane_sides(second, first);
    const auto second_sides = plane_sides(first, second);
    if (strictly_one_side(first_sides) || strictly_one_side(second_sides))
        return false;

    for (std::size_t edge = 0; edge < 3; ++edge) {
        const auto next = (edge + 1) % 3;
        if (segment_intersects(first[edge], first[next], first_sides[edge], first_sides[next], second) ||
            segment_intersects(second[edge], second[next], second_sides[edge], second_sides[next], first)) {
            return true;
        }
    }
    return false;
}

} // namespace

std::optional<Line3> intersection(const Plane3& plane1,
                                  const Plane3& plane2) {
    const Vector3 normal_1 = plane1.get_normal();
    const Vector3 normal_2 = plane2.get_normal();
    const Vector3 direction = normal_1.cross(normal_2);

    if (!Vector3::valid_vec(direction))
        return std::nullopt;

    const Vector3 op_vector = normal_1 * plane2.get_d() - normal_2 * plane1.get_d();
    const Vector3 numerator = op_vector.cross(direction);
    const double denominator = 1.0 / direction.dot(direction);
    const Vector3 supp_point = numerator * denominator;

    return Line3::Create(direction, supp_point);
}

// TODO - try/catch logic
std::optional<bool> intersection(const Triangle3& triangle_1,
                                 const Triangle3& triangle_2) {
    try {
        if (triangle_1.is_degenerate() || triangle_2.is_degenerate())
            return std::nullopt;
        const auto first_bounds = triangle_1.bounds();
        const auto second_bounds = triangle_2.bounds();
        if (!first_bounds || !second_bounds)
            return std::nullopt;
        if (!first_bounds->intersects(*second_bounds))
            return false;
        return triangles_intersect(coordinates(triangle_1), coordinates(triangle_2));
    } catch (const std::invalid_argument&) {
        return std::nullopt;
    } catch (const std::out_of_range&) {
        return std::nullopt;
    } catch (const std::runtime_error&) {
        return std::nullopt;
    }
}

} // namespace triangles
