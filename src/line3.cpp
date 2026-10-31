#include "line3.hpp"
#include "intersection.hpp"
#include "plane_utils.hpp"

namespace triangles {

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

    if (detail::is_parallel(plane1, plane2))
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

} // namespace triangles
