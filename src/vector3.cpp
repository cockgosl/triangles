#include "vector3.hpp"

#include <limits>
#include <cmath>

constexpr double EPSILON = std::numeric_limits<double>::epsilon();

Vector3::Vector3(double x, double y, double z): x(x), y(y), z(z){}

double Vector3::length() const {
    return std::sqrt(x*x + y*y + z*z);
}

Vector3 Vector3::normalized() const {
    double len = length();
    if (len < EPSILON) {
        return Vector3();
    }
    return Vector3(x/len, y/len, z/len);
}

double Vector3::dot(const Vector3& vector) const {
    return x * vector.x + y * vector.y + z * vector.z;    
}

Vector3 Vector3::cross(const Vector3& vector) const {
    return Vector3(y * vector.z - z * vector.y, z * vector.x - x * vector.z, x * vector.y - y * vector.x);
}


Vector3 Vector3::operator+(const Vector3& vector) const {
    return Vector3(x + vector.x, y + vector.y, z + vector.z);
}

Vector3 Vector3::operator-(const Vector3& vector) const {
    return Vector3(x - vector.x, y - vector.y, z - vector.z);
}

Vector3 Vector3::operator-() const {
    return Vector3(-x, -y, -z);
}

Vector3 Vector3::operator*(double scalar) const {
    return Vector3(x * scalar, y * scalar, z * scalar);
}
