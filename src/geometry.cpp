#include "geometry.hpp"
//--------------//
//Vector methods//
//--------------//
Vector3::Vector3(double x, double y, double z): x(x), y(y), z(z){}

double Vector3::length() const {
    return std::sqrt(x*x + y*y + z*z);
}

Vector3 Vector3::normalized() const {
    double len = length();
    if (len < std::numeric_limits<double>::epsilon()) {
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

Vector3 Vector3::operator*(double scalar) const {
    return Vector3(x * scalar, y * scalar, z * scalar);
}
//-------------//
//Point methods//
//-------------//
Point3::Point3(double x, double y, double z): x(x), y(y), z(z){}

Vector3 Point3::operator-(const Point3& other) const {
    return Vector3(x - other.x, y - other.y, z - other.z);
}

Point3 Point3::operator+(const Vector3& vector) const {
    return Point3(x + vector.x, y + vector.y, z + vector.z);
}

Point3 Point3::operator-(const Vector3& vector) const {
    return Point3(x - vector.x, y - vector.y, z - vector.z);
}
//----------------//
//Triangle methods//
//----------------//

Triangle3::Triangle3(const Point3& a, const Point3& b, const Point3& c): a(a), b(b), c(c){}

double Triangle3::area() const {
    return edge_AB().cross(edge_AC()).length() / 2.0;
}

bool Triangle3::is_degenerate() const {
    return area() < std::numeric_limits<double>::epsilon();
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
