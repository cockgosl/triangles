#include "triangle.hpp"

#include <limits>

namespace triangles {

constexpr double EPSILON = std::numeric_limits<double>::epsilon();


Triangle3::Triangle3(const Vector3& vector_1,
                     const Vector3& vector_2,
                     const Vector3& vector_3)
            :a_(vector_1), b_(vector_2), c_(vector_3){}
Triangle3::~Triangle3() = default;

double Triangle3::area() const {
    return edge_AB().cross(edge_AC()).length() / 2.0;
}

bool Triangle3::is_degenerate() const {
    return area() < EPSILON;
}

Vector3 Triangle3::edge_AB() const {
    return b_ - a_;
}

Vector3 Triangle3::edge_AC() const {
    return c_ - a_;
}

Vector3 Triangle3::normal() const {
    return edge_AB().cross(edge_AC()).normalized();
}

} // namespace triangles
