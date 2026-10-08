#include "triangle.hpp"

#include <limits>

constexpr double EPSILON = std::numeric_limits<double>::epsilon();


Triangle3::Triangle3(const Vector3& vector_1, const Vector3& vector_2, const Vector3& vector_3): a(vector_1), b(vector_2), c(vector_3){}

double Triangle3::area() const {
    return edge_AB().cross(edge_AC()).length() / 2.0;
}

bool Triangle3::is_degenerate() const {
    return area() < EPSILON;
}

Vector3 Triangle3::edge_AB() const {
    return b - a;
}

Vector3 Triangle3::edge_AC() const {
    return c - a;
}

Vector3 Triangle3::normal() const {
    return edge_AB().cross(edge_AC()).normalized();
}

