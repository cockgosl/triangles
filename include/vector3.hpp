#pragma once

namespace triangles {

class Vector3 {
public:
    double x;
    double y;
    double z;

    Vector3(double x = 0.0, double y = 0.0, double z = 0.0); // constructor
    ~Vector3();                                              // destructor

    double  length() const;
    double  dot(const Vector3& vector) const;
    Vector3 normalized() const;
    Vector3 cross(const Vector3& vector) const;

    Vector3 operator+(const Vector3& vector) const;
    Vector3 operator-(const Vector3& vector) const;
    Vector3 operator-() const;
    Vector3 operator*(double scalar) const;
};

} // namespace triangles
