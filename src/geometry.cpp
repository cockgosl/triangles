#include "geometry.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <optional>
#include <ranges>
#include <utility>
#include <vector>

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

    const double x_1 = std::min(point.x, endpoint.x);
    const double y_1 = std::min(point.y, endpoint.y);
    const double z_1 = std::min(point.z, endpoint.z);
    const double x_2 = std::max(point.x, endpoint.x);
    const double y_2 = std::max(point.y, endpoint.y);
    const double z_2 = std::max(point.z, endpoint.z);

    Vector3 left_bottom{x_1, y_1, z_1};
    Vector3 right_top{x_2, y_2, z_2};

    return std::pair{left_bottom, right_top};

}

// FIXME - reduce soo much coloms via using?
// comparison
bool quick_rejection(const Vector3& edge_1,
                     const Vector3& point_1,
                     const Vector3& edge_2,
                     const Vector3& point_2) {
    std::optional<std::pair<Vector3, Vector3>> bound_box_1 = make_box(edge_1, point_1);
    std::optional<std::pair<Vector3, Vector3>> bound_box_2 = make_box(edge_2, point_2);
    if (!bound_box_1 || !bound_box_2)
        return false;

    bool x_interior = (bound_box_2->second.x >= bound_box_1->first.x);
    bool y_interior = (bound_box_2->second.y >= bound_box_1->first.y);
    bool z_interior = (bound_box_2->second.z >= bound_box_1->first.z);
    bool x_endpoint = (bound_box_1->second.x >= bound_box_2->first.x);
    bool y_endpoint = (bound_box_1->second.y >= bound_box_2->first.y);
    bool z_endpoint = (bound_box_1->second.z >= bound_box_2->first.z);

    return !(x_endpoint && y_endpoint && z_endpoint &&
             x_interior && y_interior && z_interior);
}

// FIXME - comparison
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
        if (is_coincide(*plane_1, *plane_2)) {
            if (is_triangle_inclusion(triangle_1, triangle_2))
                return true;
            else if (is_edges_intersec(triangle_1, triangle_2))
                return true;
            return false;
        } else if (is_parallel(*plane_1, *plane_2))
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
