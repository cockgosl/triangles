#pragma once

#include "triangle.hpp"
#include <optional>

namespace triangles {

class Plane3;

class Line3 {
private:
    Vector3 direction_;
    Vector3 point_;

    Line3(const Vector3& direction,
          const Vector3& point);
    explicit Line3(const Vector3& direction); // ctor via default point - beginning of coordinates
public:
    static std::optional<Line3> Create(const Vector3& direction,
                                       const Vector3& point);
    static std::optional<Line3> Create(const Plane3& plane1,
                                       const Plane3& plane2);
    static std::optional<Line3> Create(const Vector3& direction);

    ~Line3();

    const Vector3& get_direction() const { return direction_; }
    const Vector3& get_point() const { return point_; }

    bool operator==(const Line3& line) const;
};


class Plane3 {
private:
    double A_;
    double B_;
    double C_;
    double D_;

    Plane3(const Vector3& point,
           const Vector3& normal);
public:
    static std::optional<Plane3> Create(const Triangle3& triangle);
    static std::optional<Plane3> Create(const Vector3& point,
                                        const Vector3& normal);
    static std::optional<Plane3> Create(const Vector3& point1,
                                        const Vector3& point2,
                                        const Vector3& point3);
    ~Plane3();

    Vector3 get_normal() const;
    double get_a() const { return A_; }
    double get_b() const { return B_; }
    double get_c() const { return C_; }
    double get_d() const { return D_; }

};

std::optional<Line3> intersection(const Plane3& plane_1,
                                  const Plane3& plane_2);
std::optional<bool> intersection(const Triangle3& triangle_1,
                                 const Triangle3& triangle_2);
                                 
class Point2 {
private:
    double x_;
    double y_;

    Point2(const double x,
           const double y);

public:
    static std::optional<Point2> Create(const Vector3& vector);
    static std::optional<Point2> Create(const double x,
                                        const double y);
    ~Point2();

    double get_x() const { return x_; }
    double get_y() const { return y_; }

};

} //namespace triangles
