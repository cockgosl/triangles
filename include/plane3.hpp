#pragma once

#include "triangle.hpp"

#include <optional>

namespace triangles {

class Plane3 {
private:
    double A_;
    double B_;
    double C_;
    double D_;

    Plane3(const Vector3& point,
           const Vector3& normal);
public:
    static std::optional<Plane3> Create(const Triangle3& triangle);
    static std::optional<Plane3> Create(const Vector3& point,
                                        const Vector3& normal);
    static std::optional<Plane3> Create(const Vector3& point1,
                                        const Vector3& point2,
                                        const Vector3& point3);
    ~Plane3();

    Vector3 get_normal() const;
    double get_a() const { return A_; }
    double get_b() const { return B_; }
    double get_c() const { return C_; }
    double get_d() const { return D_; }

};

} // namespace triangles
