#include "geometry/triangle.hpp"
#include "geometry/robust_predicates.hpp"

#include <algorithm>
#include <array>
#include <optional>

namespace triangles {

Triangle3::Triangle3(const Vector3& vector_1,
                     const Vector3& vector_2,
                     const Vector3& vector_3)
            :a(vector_1), b(vector_2), c(vector_3) {}
Triangle3::Triangle3()  = default;
Triangle3::~Triangle3() = default;

double Triangle3::area() const {
    return edge_AB().cross(edge_AC()).length() / 2.0;
}

bool Triangle3::is_degenerate() const {
    const std::array signs{
        robust::orient2d({a.x, a.y}, {b.x, b.y}, {c.x, c.y}),
        robust::orient2d({a.y, a.z}, {b.y, b.z}, {c.y, c.z}),
        robust::orient2d({a.z, a.x}, {b.z, b.x}, {c.z, c.x})};
    return std::all_of(signs.begin(), signs.end(), [](robust::Sign sign) {
        return sign == robust::Sign::zero;
    });
}

Vector3 Triangle3::edge_AB() const {
    return b - a;
}

Vector3 Triangle3::edge_AC() const {
    return c - a;
}

Vector3 Triangle3::edge_BC() const {
    return c - b;
}

Vector3 Triangle3::normal() const {
    return edge_AB().cross(edge_AC()).normalized();
}

std::optional<AABB> Triangle3::bounds() const {
    const std::array vertices{a, b, c};

    for (const auto& vertex : vertices) {
        if (!Vector3::isfinite_vec(vertex))
            return std::nullopt;
    }

    const Vector3 min{
        std::min({a.x, b.x, c.x}),
        std::min({a.y, b.y, c.y}),
        std::min({a.z, b.z, c.z})
    };

    const Vector3 max{
        std::max({a.x, b.x, c.x}),
        std::max({a.y, b.y, c.y}),
        std::max({a.z, b.z, c.z})
    };

    return AABB::Create(min, max);
}


} // namespace triangles
