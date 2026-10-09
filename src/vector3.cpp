#include "vector3.hpp"

#include <limits>
#include <cmath>

namespace triangles {

constexpr double EPSILON = std::numeric_limits<double>::epsilon();

Vector3::Vector3(double x, double y, double z)
        :x_(x), y_(y), z_(z) {}

Vector3::~Vector3() = default;

double Vector3::length() const {
    return std::sqrt(x_ * x_ + y_ * y_ + z_ * z_);
}

Vector3 Vector3::normalized() const {
    double len = length();
    if (len < EPSILON) {
        return Vector3();
    }

    return Vector3(x_ / len, y_ / len, z_ / len);
}

double Vector3::dot(const Vector3& vector) const {
    return x_ * vector.get_x() + y_ * vector.get_y() + z_ * vector.get_z();
}

Vector3 Vector3::cross(const Vector3& vector) const {
    return Vector3(y_ * vector.get_z() - z_ * vector.get_y(),
                   z_ * vector.get_x() - x_ * vector.get_z(),
                   x_ * vector.get_y() - y_ * vector.get_x());
}

Vector3 Vector3::operator+(const Vector3& vector) const {
    return Vector3(x_ + vector.get_x(),
                   y_ + vector.get_y(),
                   z_ + vector.get_z());
}

Vector3 Vector3::operator-(const Vector3& vector) const {
    return Vector3(x_ - vector.get_x(),
                   y_ - vector.get_y(),
                   z_ - vector.get_z());
}

Vector3 Vector3::operator-() const {
    return Vector3(-x_, -y_, -z_);
}

Vector3 Vector3::operator*(double scalar) const {
    return Vector3(x_ * scalar,
                   y_ * scalar,
                   z_ * scalar);
}

}
