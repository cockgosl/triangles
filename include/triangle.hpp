#pragma once

#include "vector3.hpp"

namespace triangles {

class Triangle3 {
public:
    Vector3 a;
    Vector3 b;
    Vector3 c;

    Triangle3(const Vector3& a,
              const Vector3& b,
              const Vector3& c);
    ~Triangle3();

    double area() const;
    bool   is_degenerate() const;

    Vector3 edge_AB() const;
    Vector3 edge_AC() const;
    Vector3 normal() const;
};

} // namespace triangles
