#pragma once

#include "vector3.hpp"

#include <optional>

namespace triangles {

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

} // namespace triangles
