#pragma once

#include <cmath>

class Vector3 {
    public:
        double x;
        double y;
        double z;
        
        Vector3(double x = 0.0, double y = 0.0, double z = 0.0);
        
        double length() const;
        Vector3 normalized() const;

        double dot(const Vector3& vector) const;
        Vector3 cross(const Vector3& vector) const;
        
        Vector3 operator+(const Vector3& vector) const;
        Vector3 operator-(const Vector3& vector) const;
        Vector3 operator*(double scalar) const;
};

class Point3 {
public:
    double x;
    double y;
    double z;

    Point3(double x = 0.0, double y = 0.0, double z = 0.0);

    Vector3 operator-(const Point3& other) const;
    Point3 operator+(const Vector3& vector) const;
    Point3 operator-(const Vector3& vector) const;
};

class Triangle3 {
public:
    Point3 a;
    Point3 b;
    Point3 c;

    Triangle3(const Point3& a, const Point3& b, const Point3& c);

    double area() const;
    bool is_degenerate() const;

    Vector3 edge_AB() const;
    Vector3 edge_AC() const;
    Vector3 normal() const;
};
