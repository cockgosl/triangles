#include "intersection.hpp"
#include "plane_utils.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <optional>
#include <ranges>
#include <tuple>
#include <utility>
#include <vector>

namespace triangles {

namespace {

double plane_value(const Plane3& plane, const Vector3& point) {
    return plane.get_a() * point.x +
           plane.get_b() * point.y +
           plane.get_c() * point.z +
           plane.get_d();
}

std::optional<std::pair<Vector3, Vector3>> make_box(const Vector3& edge,
                                                    const Vector3& point) {
    const Vector3 endpoint = point + edge;
    const std::array values{point.x, endpoint.x, point.y, endpoint.y, point.z, endpoint.z};
    bool is_valid = std::ranges::all_of(values, [](double elem) { return std::isfinite(elem); });
    if (!is_valid)
        return std::nullopt;

    double x_1, x_2, y_1, y_2, z_1, z_2;
    std::tie(x_1, x_2) = std::minmax(point.x, endpoint.x);
    std::tie(y_1, y_2) = std::minmax(point.y, endpoint.y);
    std::tie(z_1, z_2) = std::minmax(point.z, endpoint.z);

    Vector3 left_bottom{x_1, y_1, z_1};
    Vector3 right_top{x_2, y_2, z_2};

    return std::pair{left_bottom, right_top};

}

bool quick_rejection(const Vector3& edge_1,
                     const Vector3& point_1,
                     const Vector3& edge_2,
                     const Vector3& point_2) {
    auto bound_box_1 = make_box(edge_1, point_1);
    auto bound_box_2 = make_box(edge_2, point_2);
    if (!bound_box_1 || !bound_box_2)
        return false;

    const auto& [min1, max1] = *bound_box_1;
    const auto& [min2, max2] = *bound_box_2;

    return (max2.x < min1.x || max1.x < min2.x) ||
           (max2.y < min1.y || max1.y < min2.y) ||
           (max2.z < min1.z || max1.z < min2.z);
}

bool check_dot_sign(const Vector3& edge_1,
                    const Vector3& point_1,
                    const Vector3& edge_2,
                    const Vector3& point_2) {
    const Vector3 cross_prod_1 = (point_2 - point_1).cross(edge_1);
    const Vector3 cross_prod_2 = (point_2 + edge_2 - point_1).cross(edge_1);
    const double dot_prod = cross_prod_1.dot(cross_prod_2);

    return (dot_prod <= 0);
}

// we need guarantee, that segments lie in a same plane
bool is_segments_intersec(const Vector3& edge_1,
                          const Vector3& point_1,
                          const Vector3& edge_2,
                          const Vector3& point_2) {
    if (quick_rejection(edge_1, point_1, edge_2, point_2)) return false;

    bool sign_1 = check_dot_sign(edge_1, point_1, edge_2, point_2);
    bool sign_2 = check_dot_sign(edge_2, point_2, edge_1, point_1);

    return (sign_1 && sign_2);
}



bool is_edges_intersec(const Triangle3& triangle_1,
                       const Triangle3& triangle_2) {
    Vector3 AB_1 = triangle_1.edge_AB();
    Vector3 AC_1 = triangle_1.edge_AC();
    Vector3 BC_1 = triangle_1.edge_BC();

    Vector3 AB_2 = triangle_2.edge_AB();
    Vector3 AC_2 = triangle_2.edge_AC();
    Vector3 BC_2 = triangle_2.edge_BC();

    std::array<std::pair<Vector3, Vector3>, 3> edges_1 = {{{AB_1, triangle_1.a},
                                                           {AC_1, triangle_1.a},
                                                           {BC_1, triangle_1.b}}};

    std::array<std::pair<Vector3, Vector3>, 3> edges_2 = {{{AB_2, triangle_2.a},
                                                           {AC_2, triangle_2.a},
                                                           {BC_2, triangle_2.b}}};

    for (const auto& [edge_1, point_1] : edges_1)
        for (const auto& [edge_2, point_2] : edges_2)
            if(is_segments_intersec(edge_1, point_1, edge_2, point_2))
                return true;

    return false;
}



bool is_point_inside(const Vector3& point,
                     const Triangle3& triangle) {
    const Vector3 edge_AC = triangle.edge_AC();
    const Vector3 edge_AB = triangle.edge_AB();
    Vector3 normal = edge_AB.cross(edge_AC);

    const std::array sides {
        normal.dot(edge_AB.cross(point - triangle.a)),
        normal.dot((triangle.c - triangle.b).cross(point - triangle.b)),
        normal.dot((-edge_AC).cross(point - triangle.c)),
    };

    return std::ranges::all_of(sides, [](double sign) { return sign >= 0; });
}

bool is_triangle_inside(const Triangle3& triangle_in,
                        const Triangle3& triangle_out) {
    bool is_point_a_in = is_point_inside(triangle_in.a, triangle_out);
    bool is_point_b_in = is_point_inside(triangle_in.b, triangle_out);
    bool is_point_c_in = is_point_inside(triangle_in.c, triangle_out);

    return (is_point_a_in &&
            is_point_b_in &&
            is_point_c_in);
}

bool is_triangle_inclusion(const Triangle3& triangle_1,
                           const Triangle3& triangle_2) {
    bool is_first_in_second = is_triangle_inside(triangle_1, triangle_2);
    bool is_second_in_first = is_triangle_inside(triangle_2, triangle_1);

    return (is_first_in_second ||
            is_second_in_first);
}

// Noncoplanar triangles: cut the edges, find line parameters, compare intervals.
using LineInterval = std::pair<double, double>;

bool is_valid_triangle(const Triangle3& triangle) {
    const std::array vertices{triangle.a, triangle.b, triangle.c};
    if (!std::ranges::all_of(vertices, Vector3::isfinite_vec))
        return false;

    const Vector3 normal = triangle.edge_AB().cross(triangle.edge_AC());
    return Vector3::valid_vec(normal) && !triangle.is_degenerate();
}

bool is_valid_plane(const Plane3& plane) {
    const std::array coefficients{plane.get_a(), plane.get_b(), plane.get_c(), plane.get_d()};
    return std::ranges::all_of(coefficients, [](double value) { return std::isfinite(value); }) &&
           Vector3::valid_vec(plane.get_normal());
}

bool is_valid_line(const Line3& line) {
    return Vector3::isfinite_vec(line.get_point()) &&
           Vector3::valid_vec(line.get_direction());
}

std::optional<bool> same_side(const Plane3& plane, const Triangle3& triangle) {
    if (!is_valid_plane(plane) || !is_valid_triangle(triangle))
        return std::nullopt;

    const std::array values{plane_value(plane, triangle.a),
                            plane_value(plane, triangle.b),
                            plane_value(plane, triangle.c)};
    if (!std::ranges::all_of(values, [](double value) { return std::isfinite(value); }))
        return std::nullopt;

    return std::ranges::all_of(values, [](double value) { return value > 0.0; }) ||
           std::ranges::all_of(values, [](double value) { return value < 0.0; });
}

// An empty vector means no intersection; nullopt means invalid data or arithmetic.
std::optional<std::vector<Vector3>> edge_plane_intersections(const Vector3& point_1,
                                                             const Vector3& point_2,
                                                             const Plane3& plane) {
    if (!Vector3::isfinite_vec(point_1) || !Vector3::isfinite_vec(point_2) ||
        !is_valid_plane(plane))
        return std::nullopt;

    const double value_1 = plane_value(plane, point_1);
    const double value_2 = plane_value(plane, point_2);
    if (!std::isfinite(value_1) || !std::isfinite(value_2))
        return std::nullopt;

    std::vector<Vector3> points;
    if (value_1 == 0.0)
        points.push_back(point_1);
    if (value_2 == 0.0)
        points.push_back(point_2);
    if (!points.empty())
        return points;

    if ((value_1 < 0.0) == (value_2 < 0.0))
        return points;

    // Scale the distances instead of forming value_1 - value_2, which can overflow.
    const double distance_1 = std::fabs(value_1);
    const double distance_2 = std::fabs(value_2);
    const double ratio = std::min(distance_1, distance_2) / std::max(distance_1, distance_2);
    const double parameter = distance_1 >= distance_2 ? 1.0 / (1.0 + ratio)
                                                      : ratio / (1.0 + ratio);
    if (!std::isfinite(parameter) || parameter < 0.0 || parameter > 1.0)
        return std::nullopt;

    const Vector3 point{std::lerp(point_1.x, point_2.x, parameter),
                        std::lerp(point_1.y, point_2.y, parameter),
                        std::lerp(point_1.z, point_2.z, parameter)};
    if (!Vector3::isfinite_vec(point))
        return std::nullopt;

    points.push_back(point);
    return points;
}

// Parameter of the point's projection onto the line; the direction need not be unit.
std::optional<double> line_parameter(const Vector3& point, const Line3& line) {
    if (!Vector3::isfinite_vec(point) || !is_valid_line(line))
        return std::nullopt;

    const Vector3 offset = point - line.get_point();
    const double direction_squared = line.get_direction().dot(line.get_direction());
    if (!Vector3::isfinite_vec(offset) || !std::isfinite(direction_squared) ||
        direction_squared <= 0.0)
        return std::nullopt;

    const double parameter = offset.dot(line.get_direction()) / direction_squared;
    if (!std::isfinite(parameter))
        return std::nullopt;

    return parameter;
}

// Collect parameters of the triangle's cut by the other triangle's plane.
std::optional<std::vector<double>> triangle_line_parameters(const Triangle3& triangle,
                                                            const Plane3& other_plane,
                                                            const Line3& line) {
    if (!is_valid_triangle(triangle) || !is_valid_plane(other_plane) || !is_valid_line(line))
        return std::nullopt;

    const std::array edges{std::pair{triangle.a, triangle.b},
                           std::pair{triangle.b, triangle.c},
                           std::pair{triangle.c, triangle.a}};
    std::vector<double> parameters;
    parameters.reserve(6);

    for (const auto& [point_1, point_2] : edges) {
        const auto points = edge_plane_intersections(point_1, point_2, other_plane);
        if (!points)
            return std::nullopt;

        for (const auto& point : *points) {
            const auto parameter = line_parameter(point, line);
            if (!parameter)
                return std::nullopt;

            parameters.push_back(*parameter);
        }
    }

    return parameters;
}

// A point cut produces [t, t]; repeated parameters do not affect the bounds.
std::optional<LineInterval> make_line_interval(const std::vector<double>& parameters) {
    if (parameters.empty() ||
        !std::ranges::all_of(parameters, [](double value) { return std::isfinite(value); }))
        return std::nullopt;

    const auto [minimum, maximum] = std::ranges::minmax_element(parameters);
    return LineInterval{*minimum, *maximum};
}

std::optional<bool> intervals_overlap(const LineInterval& interval_1,
                                      const LineInterval& interval_2) {
    const std::array bounds{interval_1.first, interval_1.second,
                            interval_2.first, interval_2.second};
    if (!std::ranges::all_of(bounds, [](double value) { return std::isfinite(value); }) ||
        interval_1.first > interval_1.second || interval_2.first > interval_2.second)
        return std::nullopt;

    return std::max(interval_1.first, interval_2.first) <=
           std::min(interval_1.second, interval_2.second);
}

// Compare both triangle sections on the already computed common line.
std::optional<bool> is_line_intersec(const Triangle3& triangle_1,
                                     const Triangle3& triangle_2,
                                     const Plane3& plane_1,
                                     const Plane3& plane_2,
                                     const Line3& line) {
    const auto parameters_1 = triangle_line_parameters(triangle_1, plane_2, line);
    const auto parameters_2 = triangle_line_parameters(triangle_2, plane_1, line);
    if (!parameters_1 || !parameters_2)
        return std::nullopt;
    if (parameters_1->empty() || parameters_2->empty())
        return false;

    const auto interval_1 = make_line_interval(*parameters_1);
    const auto interval_2 = make_line_interval(*parameters_2);
    if (!interval_1 || !interval_2)
        return std::nullopt;

    return intervals_overlap(*interval_1, *interval_2);
}

} // namespace

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

std::optional<bool> intersection(const Triangle3& triangle_1,
                                 const Triangle3& triangle_2) {
    if (!is_valid_triangle(triangle_1) || !is_valid_triangle(triangle_2))
        return std::nullopt;

    const auto plane_1 = Plane3::Create(triangle_1);
    const auto plane_2 = Plane3::Create(triangle_2);
    if (!plane_1 || !plane_2 || !is_valid_plane(*plane_1) || !is_valid_plane(*plane_2))
        return std::nullopt;

    const auto intersec_line = intersection(*plane_1, *plane_2);
    if (!intersec_line) {
        if (detail::is_coincide(*plane_1, *plane_2)) {
            if (is_triangle_inclusion(triangle_1, triangle_2))
                return true;
            else if (is_edges_intersec(triangle_1, triangle_2))
                return true;
            return false;
        } else if (detail::is_parallel(*plane_1, *plane_2))
            return false;
        else
            return std::nullopt;
    }

    const auto side_1 = same_side(*plane_1, triangle_2);
    const auto side_2 = same_side(*plane_2, triangle_1);
    if (!side_1 || !side_2)
        return std::nullopt;
    if (*side_1 || *side_2)
        return false;

    return is_line_intersec(triangle_1, triangle_2, *plane_1, *plane_2, *intersec_line);
}

} // namespace triangles
