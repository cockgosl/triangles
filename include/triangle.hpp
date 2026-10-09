#pragma once

#include "vector3.hpp"

namespace triangles {

class Triangle3 {
private:
    Vector3 a_;
    Vector3 b_;
    Vector3 c_;

public:

    Triangle3(const Vector3& a,
              const Vector3& b,
              const Vector3& c);
    ~Triangle3();

    Vector3 get_a() const { return a_; }
    Vector3 get_b() const { return b_; }
    Vector3 get_c() const { return c_; }

    double area() const;
    bool   is_degenerate() const;

    Vector3 edge_AB() const;
    Vector3 edge_AC() const;
    Vector3 normal() const;
};

} // namespace triangles
