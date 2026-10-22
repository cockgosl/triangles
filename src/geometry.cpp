#include "geometry.hpp"

#include <limits>
#include <optional>

namespace triangles {

namespace {

bool is_parallel(const Plane3& plane1,
                 const Plane3& plane2) {
    Vector3 normal_1 = plane1.get_normal().normalized();
    Vector3 normal_2 = plane2.get_normal().normalized();

    if ((normal_1 == normal_2) ||
        (normal_1 == -normal_2))
        return true;

    return false;
}

bool is_coincide(const Plane3& plane1,
                 const Plane3& plane2) {
    double D_1 = plane1.get_d();
    double D_2 = plane2.get_d();

    if (is_parallel(plane1, plane2) &&
        Vector3::compare_f(D_1, D_2))
        return true;

    return false;
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
