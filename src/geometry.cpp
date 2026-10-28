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

double plane_value(const Plane3& plane, const Vector3& point) {
    return plane.get_a() * point.x +
           plane.get_b() * point.y +
           plane.get_c() * point.z +
           plane.get_d();
}

//FIXME - comparison
bool same_side(const Plane3& plane, const Triangle3& triangle) {
    double value_1 = plane_value(plane, triangle.a);
    double value_2 = plane_value(plane, triangle.b);
    double value_3 = plane_value(plane, triangle.c);

    const std::array values{value_1, value_2, value_3};

    return std::ranges::all_of(values, [](double elem) { return elem > 0; }) ||
           std::ranges::all_of(values, [](double elem) { return elem < 0; });
}

std::optional<std::pair<Vector3, Vector3>> make_box(const Vector3& edge,
                                                    const Vector3& point) {
    const double x_1 = std::min(point.x, (point + edge).x);
    const double y_1 = std::min(point.y, (point + edge).y);
    const double x_2 = std::max(point.x, (point + edge).x);
    const double y_2 = std::max(point.y, (point + edge).y);

    const std::array values{x_1, x_2, y_1, y_2};
    bool is_valid = std::ranges::any_of(values, [](double elem) { return std::isfinite(elem); });
    if (!is_valid)
        return std::nullopt;

    Vector3 left_bottom{x_1, y_1};  // FIXME how will vector initialized?
    Vector3 right_top{x_2, y_2};    // °lack of 3-rd member init

    return {left_bottom, right_top};

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

    bool x_interior = (bound_box_2->second.y >= bound_box_1->first.y);
    bool y_interior = (bound_box_2->second.x >= bound_box_1->first.x);
    bool x_endpoint = (bound_box_1->second.x >= bound_box_2->first.x);
    bool y_endpoint = (bound_box_1->second.y >= bound_box_2->first.y);

    return !(x_endpoint && y_endpoint &&
             x_interior && y_interior);
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

    std::array<std::pair<Vector3, Vector3>, 3> edges_1 = {{AB_1, triangle_1.a},
                                                          {AC_1, triangle_1.a},
                                                          {BC_1, triangle_1.b}};

    std::array<std::pair<Vector3, Vector3>, 3> edges_2 = {{AB_2, triangle_2.a},
                                                          {AC_2, triangle_2.a},
                                                          {BC_2, triangle_2.b}};

    for (const auto& [edge_1, point_1] : edges_1)
        for (const auto& [edge_2, point_2] : edges_2)
            if(is_segments_intersec(edge_1, point_1, edge_2, point_2))
                return true;

    return false;
}



bool is_point_inside(const Vector3& point,
                     const Triangle3& triangle) {
    const Vector3 edge_AC = triangle.edge_AC();
    const Vector3 edge_BA = triangle.edge_AB();
    const Vector3 edge_BC = ;
    Vector3 normal = edge_BA.cross(edge_AC);

    const std::array sides {
        normal.dot(edge_AB.cross(point - triangle.a)),
        normal.dot((triangle.c - triangle.b).cross(point - triangle.b)),
        normal.dot((-edge_AC).cross(point - triangle.c)),
    };

    return std::ranges::all_of(sides, [](double sign) { return sign >= 0; });
}

bool is_triangle_inside(const Triangle3& triangle_in
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
    bool is_first_in_second = is_triagle_inside(triangle_1, triangle_2);
    bool is_second_in_first = is_triagle_inside(triangle_2, triangle_1);

    return (is_first_in_second ||
            is_second_in_first);
}

std::optional<bool> intersection(const Triangle3& triangle_1,
                                 const Triangle3& triangle_2) {
    Plane3 plane_1 = Plane3::Create(triangle_1);
    Plane3 plane_2 = Plane3::Create(triangle_2);
    Line3 intersec_line = intersection(plane_1, plane_2);
    if (!intersec_line) {
        if (is_coincide(plane_1, plane_2)) {
            if (is_triangle_inclusion(triangle_1, triangle_2)) {
                return true;
            } else if (/* edges_intersect */) {
                return true;
            } else
                return false;

        } else if (is_parallel(plane_1, plane_2)) {
            return false;
        } else
            return std::nullopt;
    }

    if (same_side(plane_1, triangle_2))
        return false;

    /* then start's handling of 3rd situation */

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

std::optional<Point2> Point2::create(const double x,
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
