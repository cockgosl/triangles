#include "geometry/plane3.hpp"
#include "geometry/robust_predicates.hpp"
#include "geometry/vector3.hpp"

#include <stdexcept>
#include <cmath>

namespace triangles {

namespace {

bool valid_triangle(const Triangle3& triangle) {
    try {
        return !triangle.is_degenerate();
    } catch (const std::invalid_argument&) {
        return false;
    } catch (const std::out_of_range&) {
        return false;
    } catch (const std::runtime_error&) {
        return false;
    }
}

} // namespace


bool Plane3::is_parallel(const Plane3& plane1,
                         const Plane3& plane2) {
    const Vector3 n1 = plane1.get_normal();
    const Vector3 n2 = plane2.get_normal();

    if (!(Vector3::valid_vec(n1) && Vector3::valid_vec(n2))) return false; 

    const auto xy = robust::orient2d({n1.x, n1.y}, 
                                     {n2.x, n2.y}, 
                                     {0.0 , 0.0 });
    const auto xz = robust::orient2d({n1.x, n1.z}, 
                                     {n2.x, n2.z}, 
                                     {0.0 , 0.0 });

    const auto yz = robust::orient2d({n1.y, n1.z}, 
                                     {n2.y, n2.z}, 
                                     {0.0 , 0.0 });

    return xy == robust::Sign::zero &&
           xz == robust::Sign::zero &&
           yz == robust::Sign::zero;
}

bool Plane3::is_coincide(const Plane3& plane1,
                         const Plane3& plane2) {
    const Vector3 n1 = plane1.get_normal();
    const Vector3 n2 = plane2.get_normal();

    if (!(Vector3::valid_vec(n1) && Vector3::valid_vec(n2))) return false; 


    const auto d1 = plane1.get_d();
    const auto d2 = plane2.get_d();

    const auto ad = robust::orient2d({n1.x, d1}, 
                                     {n2.x, d2}, 
                                     {0.0, 0.0});

    const auto bd = robust::orient2d({n1.y, d1}, 
                                     {n2.y, d2}, 
                                     {0.0, 0.0});

    const auto cd = robust::orient2d({n1.z, d1}, 
                                     {n2.z, d2}, 
                                     {0.0, 0.0});

    return is_parallel(plane1, plane2) &&
           ad == robust::Sign::zero &&
           bd == robust::Sign::zero &&
           cd == robust::Sign::zero;
}

std::optional<Plane3> Plane3::Create(const Triangle3& triangle) {
    if (!valid_triangle(triangle))
        return std::nullopt;

    return Plane3::Create(triangle.a, -triangle.normal());
}

std::optional<Plane3> Plane3::Create(const Vector3& point1,
                                     const Vector3& point2,
                                     const Vector3& point3) {
    const Triangle3 triangle(point1, point2, point3);

    if (!valid_triangle(triangle))
        return std::nullopt;

    return Plane3::Create(point3, triangle.normal());
}

std::optional<Plane3> Plane3::Create(const Vector3& point,
                                     const Vector3& normal) {
    if (!(Vector3::valid_vec(normal)))
        return std::nullopt;

    if (!(Vector3::isfinite_vec(point)))
        return std::nullopt;

    const Plane3 plane(point, normal);

    if (!std::isfinite(plane.get_d()))
        return std::nullopt;
    
    return plane;
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
