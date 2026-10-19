#include "geometry.hpp"

#include <limits>

namespace triangles {

constexpr double EPSILON = std::numeric_limits<double>::epsilon();

Line3::Line3(const Vector3& direction,
             const Vector3& point)
        : direction_(direction.normalized()),
          point_(point) {}

Line3::Line3(const Vector3& direction)
    : Line3(direction, Vector3{0.0, 0.0, 0.0}) {}

Line3::Line3(const Plane3& plane1,
             const Plane3& plane2) {
    bool is_crossed = plane1.is_crossed(plane2);
    if (!is_crossed) {
        direction_ = {0.0, 0.0, 0.0};
        point_ = {0.0, 0.0, 0.0};

    }

    const Vector3 normal_1 = plane1.get_normal();
    const Vector3 normal_2 = plane2.get_normal();
    Vector3 direction = normal_1.cross(normal_2);
    Vector3 point = //TODO - how to get a point og beggining of line?

}

Line3::~Line3() = default;

bool Line3::operator==(const Line3& line) const { //FIXME
    return (direction_ == line.get_direction() &&
            point_     == line.get_point());
}


// TODO - draft
bool Line3::is_crossover(const Triangle3 triangle) {
    Plane3  line_plane = Plane3(triangle);
    Vector3 rel_vec = point_ - line_plane.get_point();

    double par_x = ((rel_vec.x) * line_plane.get_normal().x) / (direction_.x * line_plane.get_normal().x);
    double par_y = ((rel_vec.y) * line_plane.get_normal().y) / (direction_.y * line_plane.get_normal().y);
    double par_z = ((rel_vec.z) * line_plane.get_normal().z) / (direction_.z * line_plane.get_normal().z);
    if (!compare_f(par_x, par_y) ||
        !compare_f(par_x, par_z) ||
        !compare_f(par_y, par_z)) return false;

    double par = (par_x + par_y + par_z) / 3;
    bool is_crossed = compare_f(line_plane.get_normal().dot(rel_vec + direction_ * par), 0.0);

    return is_crossed;
    /* Build appropriate plane for triagnle
       point_ + direction_ * t == */
}

std::optional<Line3> Plane3::intersection(const Plane3& plane1,
                                  const Plane3& plane2) {

}


Plane3::Plane3(const Triangle3& triangle)
    : Plane3(triangle.c, triangle.a.cross(triangle.b)) {}

Plane3::Plane3(const Vector3& point1,
               const Vector3& point2,
               const Vector3& point3)
    : Plane3(point3, (point1 - point3).cross(point2 - point3)) {}

Plane3::Plane3(const Vector3& point,
               const Vector3& normal) {
    A_ = normal.normalized().x;
    B_ = normal.normalized().y;
    C_ = normal.normalized().z;
    D_ = -(normal.normalized().dot(point));
}

const Vector3& Plane3::get_normal() {
    const Vector3 normal{A_, B_, C_};
    return normal;
}




} // namespace triangles
