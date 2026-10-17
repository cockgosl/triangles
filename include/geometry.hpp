#pragma once

#include "triangle.hpp"
#include <optional>

namespace triangles {

class Plane3;

class Line3 {
private:
    Vector3 direction_;
    Vector3 point_;

public:
    Line3(const Vector3& direction,
          const Vector3& point);
    Line3(const Plane3& plane1,
          const Plane3& plane2);
    explicit Line3(const Vector3& direction); // ctor via default point - beginning of coordinates
    ~Line3();

    const Vector3& get_direction() const { return direction_; }
    const Vector3& get_point() const { return point_; }

    bool is_crossover(const Triangle3& triangle) const;
    bool operator==(const Line3& line) const;
};


class Plane3 {
private:
    double A_;
    double B_;
    double C_;
    double D_;

public:
    explicit Plane3(const Triangle3& triangle);
    Plane3(const Vector3& point,
           const Vector3& normal);
    Plane3(const Vector3& point1,
           const Vector3& point2,
           const Vector3& point3);
    ~Plane3();

    Vector3 get_normal() const;
    Vector3 get_point() const;

    std::optional<Line3> intersection(const Plane3&, const Plane3&);

};


} //namespace triangles
