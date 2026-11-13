#include "geometry/line3.hpp"
#include "geometry/intersection.hpp"
#include "geometry/robust_predicates.hpp"

namespace triangles {

namespace {

bool parallel_directions(const Vector3& d1,
                         const Vector3& d2) {
    const auto xy = robust::orient2d({d1.x, d1.y},
                                     {d2.x, d2.y},
                                     {0.0 , 0.0 });

    const auto xz = robust::orient2d({d1.x, d1.z},
                                     {d2.x, d2.z},
                                     {0.0 , 0.0 });

    const auto yz = robust::orient2d({d1.y, d1.z},
                                     {d2.y, d2.z},
                                     {0.0, 0.0 });

    return xy == robust::Sign::zero &&
           xz == robust::Sign::zero &&
           yz == robust::Sign::zero;
}

bool point_on_line(const Vector3& point,
                   const Vector3& origin,
                   const Vector3& direction) {
    const auto xy = robust::orient3d({origin.x   , origin.y   , 1.0},
                                     {point.x    , point.y    , 1.0},
                                     {direction.x, direction.y, 0.0},
                                     {0.0        , 0.0        , 0.0});

    const auto xz = robust::orient3d({origin.x   , origin.z   , 1.0},
                                     {point.x    , point.z    , 1.0},
                                     {direction.x, direction.z, 0.0},
                                     {0.0        , 0.0        , 0.0});

    const auto yz = robust::orient3d({origin.y   , origin.z   , 1.0},
                                     {point.y    , point.z    , 1.0},
                                     {direction.y, direction.z, 0.0},
                                     {0.0        , 0.0        , 0.0});

    return xy == robust::Sign::zero &&
           xz == robust::Sign::zero &&
           yz == robust::Sign::zero;
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

    if (Plane3::is_parallel(plane1, plane2))
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

bool Line3::operator==(const Line3& line) const {
    const bool directions_match = parallel_directions(direction_, line.direction_);
    const bool point_belongs = point_on_line(line.point_, point_, direction_);

    return directions_match && point_belongs;
}

} // namespace triangles
