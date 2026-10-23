#include "geometry.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <optional>
#include <ranges>
#include <utility>

namespace triangles {

namespace {

std::optional<double> parallel_coefficient_ratio(const Plane3& plane1,
                                                const Plane3& plane2) {
    const std::array coefficients{
        std::pair{plane1.get_a(), plane2.get_a()},
        std::pair{plane1.get_b(), plane2.get_b()},
        std::pair{plane1.get_c(), plane2.get_c()},
        std::pair{plane1.get_d(), plane2.get_d()}};

    const bool all_finite = std::ranges::all_of(coefficients, [](const auto& pair) {
        return std::isfinite(pair.first) && std::isfinite(pair.second);
    });

    if (!all_finite)
        return std::nullopt;

    const auto normal_coefficients = coefficients | std::views::take(3);
    const bool first_has_normal = std::ranges::any_of(
        normal_coefficients, [](const auto& pair) { return !Vector3::compare_f(pair.first, 0.0); });

    const auto pivot = std::ranges::max_element(
        normal_coefficients, {}, [](const auto& pair) {
            return std::fabs(pair.second);
        });

    if (!first_has_normal || Vector3::compare_f(pivot->second, 0.0))
        return std::nullopt;


    // Use the largest normal coefficient as the only divisor
    const double ratio = pivot->first / pivot->second;
    if (!std::isfinite(ratio) || Vector3::compare_f(ratio, 0.0))
        return std::nullopt;

    for (const auto& [first, second] : normal_coefficients) {
        const double scaled = ratio * second;
        if (!std::isfinite(scaled) || !Vector3::compare_f(first, scaled))
            return std::nullopt;

    }
    return ratio;
}

bool is_parallel(const Plane3& plane1,
                 const Plane3& plane2) {
    return parallel_coefficient_ratio(plane1, plane2).has_value();
}

bool is_coincide(const Plane3& plane1,
                 const Plane3& plane2) {
    const auto ratio = parallel_coefficient_ratio(plane1, plane2);
    if (!ratio)
        return false;

    const double scaled_d = *ratio * plane2.get_d();
    return std::isfinite(scaled_d) &&
           Vector3::compare_f(plane1.get_d(), scaled_d);
}

} // namespace


std::optional<Line3> Line3::Create(const Vector3& direction,
                                   const Vector3& point) {
    if (!(Vector3::valid_vec(direction)))
        return std::nullopt;

    if (!(Vector3::isfinite_vec(point)))
        return std::nullopt;

    return Line3(direction, point);
}

std::optional<Line3> Line3::Create(const Plane3& plane1,
                                   const Plane3& plane2) {
    if (!(Vector3::valid_vec(plane1.get_normal())))
        return std::nullopt;

    if (!(Vector3::valid_vec(plane2.get_normal())))
        return std::nullopt;

    if (is_parallel(plane1, plane2))
        return std::nullopt;

    return intersection(plane1, plane2);
}

std::optional<Line3> Line3::Create(const Vector3& direction) {
    return Create(direction, Vector3{0.0, 0.0, 0.0});
}

Line3::Line3(const Vector3& direction,
             const Vector3& point)
        : direction_(direction.normalized()),
          point_(point) {}

Line3::~Line3() = default;

bool Line3::operator==(const Line3& line) const { //FIXME
    const Vector3 this_direction = direction_.normalized();
    const Vector3 line_direction = line.direction_.normalized();

    bool is_collinear = (this_direction.cross(line_direction) == Vector3(0.0, 0.0, 0.0));
    Vector3 op_vec = (line.get_point() - point_).cross(line_direction); // "op" - abbreviation from operand
    bool is_point_belong = !(Vector3::valid_vec(op_vec));

    return (is_collinear && is_point_belong);
}


std::optional<Line3> intersection(const Plane3& plane1,
                                  const Plane3& plane2) {
    const Vector3 normal_1 = plane1.get_normal().normalized();
    const Vector3 normal_2 = plane2.get_normal().normalized();
    const Vector3 direction = normal_1.cross(normal_2);

    if (!Vector3::valid_vec(direction))
        return std::nullopt;

    const Vector3 op_vector = normal_1 * plane2.get_d() - normal_2 * plane1.get_d();
    const Vector3 numerator = op_vector.cross(direction);
    const double denominator = 1.0 / direction.dot(direction);

    const Vector3 supp_point = numerator * denominator; // "supp" - abbreviation from supportive

    return Line3::Create(direction.normalized(), supp_point);
}


std::optional<Plane3> Plane3::Create(const Triangle3& triangle) {
    if(triangle.is_degenerate())
        return std::nullopt;

    Vector3 edge3 = triangle.a;
    Vector3 edge2 = triangle.edge_AB();
    Vector3 edge1 = triangle.edge_AC();
    Vector3 normal = edge1.cross(edge2);

    return Plane3::Create(edge3, normal);
}

std::optional<Plane3> Plane3::Create(const Vector3& point1,
                                     const Vector3& point2,
                                     const Vector3& point3) {
    Vector3 edge_1 = point1 - point3;
    Vector3 edge_2 = point2 - point3;
    Vector3 normal = edge_1.cross(edge_2);

    return Plane3::Create(point3, normal);
}

std::optional<Plane3> Plane3::Create(const Vector3& point,
                                     const Vector3& normal) {
    if (!(Vector3::valid_vec(normal)))
        return std::nullopt;

    if (!(Vector3::isfinite_vec(point)))
        return std::nullopt;

    return Plane3(point, normal);
}

Plane3::Plane3(const Vector3& point,
               const Vector3& normal) {
    A_ = normal.normalized().x;
    B_ = normal.normalized().y;
    C_ = normal.normalized().z;
    D_ = -(normal.normalized().dot(point));
}

Plane3::~Plane3() = default;

Vector3 Plane3::get_normal() const {
    return Vector3{A_, B_, C_};
}

} // namespace triangles
