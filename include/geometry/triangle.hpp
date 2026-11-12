#ifndef TRIANGLE3_HPP_
#define TRIANGLE3_HPP_

#include "vector3.hpp"
#include "aabb.hpp"

#include <cstddef>
#include <iosfwd>
#include <optional>
#include <string>
#include <vector>

namespace triangles {

class Triangle3 {
public:
    Vector3 a;
    Vector3 b;
    Vector3 c;

    Triangle3();
    Triangle3(const Vector3& a,
              const Vector3& b,
              const Vector3& c);
    ~Triangle3();

    double area() const;
    bool is_degenerate() const; // Exact collinearity, invalid coordinates or arithmetic modes throw

    Vector3 edge_AB() const;
    Vector3 edge_AC() const;
    Vector3 edge_BC() const;
    Vector3 normal() const;
    std::optional<AABB> bounds() const;
};

struct SceneTriangle {
    Triangle3 triangle;
    AABB bounds;
};

inline constexpr std::size_t max_triangle_count = 999999;

[[nodiscard]] std::optional<std::size_t> read_triangle_count(std::istream& input,
                                                             std::string* error = nullptr);
[[nodiscard]] std::optional<std::vector<SceneTriangle>> read_triangles(std::istream& input,
                                                                       std::size_t count,
                                                                       std::string* error = nullptr);

} // namespace triangles

#endif //TRIANGLE3_HPP_
