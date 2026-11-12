#include "geometry/vector3.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace triangles {

Vector3::Vector3(double x, double y, double z)
        :x(x), y(y), z(z) {}

Vector3::~Vector3() = default;

double Vector3::length() const {
    return std::hypot(x, y, z);
}

Vector3 Vector3::normalized() const {
    if (!valid_vec(*this))
        throw std::invalid_argument("Cannot normalize a zero or nonfinite vector.");

    double max = std::max({std::fabs(x), std::fabs(y), std::fabs(z)});
    Vector3 ir_vec = Vector3(x / max, y / max, z / max); // ir = intermediate
    double len = ir_vec.length();

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

bool Vector3::isfinite_vec(const Vector3& vector) {
    if (!std::isfinite(vector.x) ||
        !std::isfinite(vector.y) ||
        !std::isfinite(vector.z))
        return false;

    return true;
}

bool Vector3::operator==(const Vector3& vector) const {
    if (!isfinite_vec(vector)) return false;

    return (x == vector.x &&
            y == vector.y &&
            z == vector.z);
}

bool Vector3::valid_vec(const Vector3& vector) {
    if (!isfinite_vec(vector)) return false;

    return !(vector == Vector3(0.0, 0.0, 0.0));
}

} // namespace triangles
