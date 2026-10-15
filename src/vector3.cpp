#include "vector3.hpp"

#include <limits>
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace triangles {

constexpr double EPSILON = std::numeric_limits<double>::epsilon();

/*  TODO - move && create into general_utils file
    inconsistent logic - in vector3.cpp (lines 30-32) comparison
    we should divide approx compare & precise compare */
bool compare_f(double first, double second) {
    return (std::fabs(first - second) < EPSILON);
}


Vector3::Vector3(double x, double y, double z)
        :x(x), y(y), z(z) {}

Vector3::~Vector3() = default;

double Vector3::length() const {
    return std::sqrt(x * x + y * y + z * z);
}

Vector3 Vector3::normalized() const {
    if (compare_f(x, 0.0) &&
        compare_f(y, 0.0) &&
        compare_f(z, 0.0)) throw std::invalid_argument("attemp normalizing null-vector");

    double max = std::max({std::fabs(x), std::fabs(y), std::fabs(z)});
    Vector3 ir_vec = Vector3(x / max, y / max, z / max); // ir = intermediate

    double len = ir_vec.length();
    if (len < EPSILON) {    // FIXME - unnecessary check
        return Vector3();
    }

    return Vector3(ir_vec.x / len, ir_vec.y / len, ir_vec.z / len);
}

double Vector3::dot(const Vector3& vector) const {
    return x * vector.x + y * vector.y + z * vector.z;
}

Vector3 Vector3::cross(const Vector3& vector) const {
    return Vector3(y * vector.z - z * vector.y,
                   z * vector.x - x * vector.z,
                   x * vector.y - y * vector.x);
}

Vector3 Vector3::operator+(const Vector3& vector) const {
    return Vector3(x + vector.x,
                   y + vector.y,
                   z + vector.z);
}

Vector3 Vector3::operator-(const Vector3& vector) const {
    return Vector3(x - vector.x,
                   y - vector.y,
                   z - vector.z);
}

Vector3 Vector3::operator-() const {
    return Vector3(-x, -y, -z);
}

Vector3 Vector3::operator*(double scalar) const {
    return Vector3(x * scalar,
                   y * scalar,
                   z * scalar);
}

bool Vector3::operator==(const Vector3 vector) {
    return (x == vector.x &&
            y == vector.y &&
            z == vector.z);
}

} // namespace triangles
