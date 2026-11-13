#include "geometry/triangle.hpp"
#include "geometry/robust_predicates.hpp"

#include <algorithm>
#include <array>
#include <charconv>
#include <exception>
#include <istream>
#include <string>
#include <system_error>
#include <vector>
#include <optional>

namespace triangles {

namespace {

bool read_coordinate(std::istream& input, double& coordinate) {
    std::string token;
    if (!(input >> token))
        return false;

    const char* begin = token.data();
    const char* end = begin + token.size();
    if (*begin == '+') {
        ++begin;
        if (begin == end || *begin == '+' || *begin == '-')
            return false;
    }
    const auto [parsed_end, status] = std::from_chars(begin, end, coordinate);
    return status == std::errc{} && parsed_end == end;
}

Vector3 triangle_area_vector(const Triangle3& triangle) {
    const auto result = robust::triangle_cross(
        {triangle.a.x, triangle.a.y, triangle.a.z},
        {triangle.b.x, triangle.b.y, triangle.b.z},
        {triangle.c.x, triangle.c.y, triangle.c.z});

    return {result[0], result[1], result[2]};
}

} // namespace

Triangle3::Triangle3(const Vector3& vector_1,
                     const Vector3& vector_2,
                     const Vector3& vector_3)
            :a(vector_1), b(vector_2), c(vector_3) {}
Triangle3::Triangle3()  = default;
Triangle3::~Triangle3() = default;

double Triangle3::area() const {
    return triangle_area_vector(*this).length() / 2.0;
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
    return triangle_area_vector(*this).normalized();
}

std::optional<AABB> Triangle3::bounds() const {
    const std::array vertices{a, b, c};

    for (const auto& vertex : vertices)
        if (!Vector3::isfinite_vec(vertex))
            return std::nullopt;

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


std::optional<std::size_t> read_triangle_count(std::istream& input,
                                               std::string* error) {
    if (error)
        error->clear();

    std::string token;
    std::size_t count = 0;
    if (input >> token) {
        const auto [end, status] = std::from_chars(token.data(), token.data() + token.size(), count);
        if (status == std::errc{} && end == token.data() + token.size() && count > 0 && count <= max_triangle_count)
            return count;
    }

    if (error)
        *error = "Expected an integer N with 0 < N < 1000000.";
    return std::nullopt;
}

std::optional<std::vector<SceneTriangle>> read_triangles(std::istream& input,
                                                         std::size_t count,
                                                         std::string* error) {
    if (error)
        error->clear();

    const auto fail = [error](const std::string& message) -> std::optional<std::vector<SceneTriangle>> {
        if (error) *error = message;
            return std::nullopt;
    };

    if (count == 0 || count > max_triangle_count)
        return fail("Expected an integer N with 0 < N < 1000000.");

    std::vector<SceneTriangle> result;
    result.reserve(count);

    for (std::size_t index = 0; index < count; ++index) {
        const std::string prefix = "Triangle " + std::to_string(index + 1) + ": ";
        Triangle3 triangle;
        for (Vector3* point : {&triangle.a, &triangle.b, &triangle.c}) {
            if (!read_coordinate(input, point->x) || !read_coordinate(input, point->y) ||
                !read_coordinate(input, point->z))
                return fail(prefix + "expected nine numeric coordinates.");
        }

        const auto bounds = triangle.bounds();
        if (!bounds)
            return fail(prefix + "coordinates must be finite.");

        try {
            if (triangle.is_degenerate()) return fail(prefix + "vertices must not be collinear.");
        } catch (const std::exception& exception) {
            return fail(prefix + exception.what());
        }

        result.push_back(SceneTriangle{triangle, *bounds});
    }

    return result;
}

} // namespace triangles
