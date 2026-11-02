#ifndef GENERATOR_HPP_
#define GENERATOR_HPP_

#include "aabb.hpp"
#include "triangle.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <random>
#include <vector>

namespace triangles {

struct GeneratorConfig {
    AABB domain;

    double target_max_edge_length{1.0};
    double relative_size_variation{0.1};

    double min_shape_quality{0.1};

    std::uint32_t seed{5489u};

    std::size_t max_shape_attempts{1000};
};

struct GeneratedTriangle {
    Triangle3 triangle;
    AABB bounds;
};

class Generator {
  private:
    AABB domain_;
    AABB centroid_domain_;

    double min_edge_length_;
    double max_edge_length_;
    double min_shape_quality_;

    std::size_t max_shape_attempts_;

    std::mt19937 gen_;

    Generator(const GeneratorConfig &config, const AABB &centroid_domain,
              double min_edge_length, double max_edge_length);

    std::optional<Triangle3> generate_local_triangle();
    std::optional<Triangle3>
    generate_random_orientation(const Triangle3 &triangle);

  public:
    static std::optional<Generator> Create(const GeneratorConfig &config,
                                           std::string *error = nullptr);

    std::optional<GeneratedTriangle> generate_triangle();

    std::optional<std::vector<GeneratedTriangle>>
    generate_triangles(std::size_t count);
};

} // namespace triangles

#endif // GENERATOR_HPP_
