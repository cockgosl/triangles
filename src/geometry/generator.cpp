#include "geometry/generator.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <numbers>

namespace triangles {

Generator::Generator(const GeneratorConfig &config, const AABB &centroid_domain,
                     double min_edge_length, double max_edge_length)
    : domain_(config.domain), centroid_domain_(centroid_domain),
      min_edge_length_(min_edge_length), max_edge_length_(max_edge_length),
      min_shape_quality_(config.min_shape_quality),
      max_shape_attempts_(config.max_shape_attempts), gen_(config.seed) {}

std::optional<Generator> Generator::Create(const GeneratorConfig &config,
                                           std::string *error) {
    if (error) {
        error->clear();
    }

    const auto fail = [error](const char *message) -> std::optional<Generator> {
        if (error) {
            *error = message;
        }

        return std::nullopt;
    };

    const double max_shape_quality = std::sqrt(3.0) / 2.0;

    const double target = config.target_max_edge_length;
    const double variation = config.relative_size_variation;
    const double quality = config.min_shape_quality;

    if (!std::isfinite(target) || target <= 0.0) {
        return fail("Target edge length must be finite and positive.");
    }

    if (!std::isfinite(variation) || variation < 0.0 || variation >= 1.0) {
        return fail("Relative size variation must be finite and in [0, 1).");
    }

    if (!std::isfinite(quality) || quality <= 0.0 ||
        quality >= max_shape_quality) {
        return fail("Shape quality must be finite and in (0, sqrt(3) / 2).");
    }

    if (config.max_shape_attempts == 0) {
        return fail("Maximum shape attempts must be positive.");
    }

    const double min_edge_length = target * (1.0 - variation);
    const double max_edge_length = target * (1.0 + variation);

    if (!std::isfinite(min_edge_length) || !std::isfinite(max_edge_length) ||
        min_edge_length <= 0.0) {
        return fail("Computed edge length range is invalid.");
    }

    const double min_area = quality * min_edge_length * min_edge_length / 2.0;

    if (!std::isfinite(min_area) ||
        min_area <= std::numeric_limits<double>::epsilon()) {
        return fail("Minimum triangle area is too small or non-finite.");
    }

    const Vector3 domain_min = config.domain.get_min();
    const Vector3 domain_max = config.domain.get_max();
    const Vector3 domain_extent = config.domain.extent();

    const std::array extents{domain_extent.x, domain_extent.y, domain_extent.z};

    for (const double extent : extents) {
        if (!std::isfinite(extent) || extent <= 0.0) {
            return fail("Domain extents must be finite and positive.");
        }
    }

    const double coordinate_scale =
        std::max({1.0, std::abs(domain_min.x), std::abs(domain_min.y),
                  std::abs(domain_min.z), std::abs(domain_max.x),
                  std::abs(domain_max.y), std::abs(domain_max.z)});

    const double safety_margin =
        16.0 * std::numeric_limits<double>::epsilon() * coordinate_scale;

    const double geometric_margin = 2.0 * max_edge_length / 3.0;

    const double margin = geometric_margin + safety_margin;

    if (!std::isfinite(margin)) {
        return fail("Computed domain margin is non-finite.");
    }

    const Vector3 centroid_min{domain_min.x + margin, domain_min.y + margin,
                               domain_min.z + margin};

    const Vector3 centroid_max{domain_max.x - margin, domain_max.y - margin,
                               domain_max.z - margin};

    if (!Vector3::isfinite_vec(centroid_min) ||
        !Vector3::isfinite_vec(centroid_max)) {
        return fail("Computed centroid bounds are non-finite.");
    }

    if (centroid_min.x >= centroid_max.x || centroid_min.y >= centroid_max.y ||
        centroid_min.z >= centroid_max.z) {
        return fail("Domain is too small for the requested triangle size.");
    }

    const auto centroid_domain = AABB::Create(centroid_min, centroid_max);

    if (!centroid_domain) {
        return fail("Failed to create the centroid domain.");
    }

    return Generator(config, *centroid_domain, min_edge_length,
                     max_edge_length);
}

std::optional<Triangle3> Generator::generate_local_triangle() {
    std::uniform_real_distribution<double> coordinate_distribution(-1.0, 1.0);

    std::uniform_real_distribution<double> edge_distribution(min_edge_length_,
                                                             max_edge_length_);

    for (std::size_t attempt = 0; attempt < max_shape_attempts_; ++attempt) {
        const Vector3 a{coordinate_distribution(gen_),
                        coordinate_distribution(gen_), 0.0};

        const Vector3 b{coordinate_distribution(gen_),
                        coordinate_distribution(gen_), 0.0};

        const Vector3 c{coordinate_distribution(gen_),
                        coordinate_distribution(gen_), 0.0};

        const Vector3 ab = b - a;
        const Vector3 ac = c - a;
        const Vector3 bc = c - b;

        const double ab_squared = ab.dot(ab);
        const double ac_squared = ac.dot(ac);
        const double bc_squared = bc.dot(bc);

        const double max_edge_squared =
            std::max({ab_squared, ac_squared, bc_squared});

        if (!std::isfinite(max_edge_squared) || max_edge_squared <= 0.0) {
            continue;
        }

        const double twice_area = std::abs(ab.cross(ac).z);

        const double quality = twice_area / max_edge_squared;

        if (!std::isfinite(quality) || quality < min_shape_quality_) {
            continue;
        }

        const double source_max_edge_length = std::sqrt(max_edge_squared);

        const double target_max_edge_length = edge_distribution(gen_);

        const double scale = target_max_edge_length / source_max_edge_length;

        if (!std::isfinite(scale)) {
            continue;
        }

        const Vector3 centroid{(a.x + b.x + c.x) / 3.0, (a.y + b.y + c.y) / 3.0,
                               0.0};

        const Triangle3 triangle{(a - centroid) * scale, (b - centroid) * scale,
                                 (c - centroid) * scale};

        if (triangle.is_degenerate()) {
            continue;
        }

        return triangle;
    }

    return std::nullopt;
}

std::optional<Triangle3>
Generator::generate_random_orientation(const Triangle3 &triangle) {
    std::uniform_real_distribution<double> z_distribution(-1.0, 1.0);

    std::uniform_real_distribution<double> angle_distribution(
        0.0, 2.0 * std::numbers::pi_v<double>);

    const double z = z_distribution(gen_);
    const double azimuth = angle_distribution(gen_);

    const double radius = std::sqrt(std::max(0.0, 1.0 - z * z));

    const Vector3 normal{radius * std::cos(azimuth), radius * std::sin(azimuth),
                         z};

    const Vector3 reference = std::abs(normal.z) < 0.9 ? Vector3{0.0, 0.0, 1.0}
                                                       : Vector3{0.0, 1.0, 0.0};

    const Vector3 raw_u = reference.cross(normal);

    if (!Vector3::valid_vec(raw_u)) {
        return std::nullopt;
    }

    const Vector3 u = raw_u.normalized();
    const Vector3 v = normal.cross(u);

    const double rotation_angle = angle_distribution(gen_);

    const double cos_angle = std::cos(rotation_angle);
    const double sin_angle = std::sin(rotation_angle);

    const Vector3 rotated_u = u * cos_angle + v * sin_angle;

    const Vector3 rotated_v = u * (-sin_angle) + v * cos_angle;

    const auto rotate_point = [&](const Vector3 &point) {
        return rotated_u * point.x + rotated_v * point.y + normal * point.z;
    };

    const Triangle3 result{rotate_point(triangle.a), rotate_point(triangle.b),
                           rotate_point(triangle.c)};

    if (!Vector3::isfinite_vec(result.a) || !Vector3::isfinite_vec(result.b) ||
        !Vector3::isfinite_vec(result.c)) {
        return std::nullopt;
    }

    return result;
}

std::optional<GeneratedTriangle> Generator::generate_triangle() {
    auto local_triangle = generate_local_triangle();

    if (!local_triangle) {
        return std::nullopt;
    }

    auto rotated_triangle = generate_random_orientation(*local_triangle);

    if (!rotated_triangle) {
        return std::nullopt;
    }

    const Vector3 domain_min = centroid_domain_.get_min();
    const Vector3 domain_max = centroid_domain_.get_max();

    std::uniform_real_distribution<double> x_distribution(domain_min.x,
                                                          domain_max.x);

    std::uniform_real_distribution<double> y_distribution(domain_min.y,
                                                          domain_max.y);

    std::uniform_real_distribution<double> z_distribution(domain_min.z,
                                                          domain_max.z);

    const Vector3 centroid{x_distribution(gen_), y_distribution(gen_),
                           z_distribution(gen_)};

    const Triangle3 triangle{rotated_triangle->a + centroid,
                             rotated_triangle->b + centroid,
                             rotated_triangle->c + centroid};

    const auto bounds = triangle.bounds();

    if (!bounds) {
        return std::nullopt;
    }

    if (!domain_.contains(*bounds)) {
        return std::nullopt;
    }

    return GeneratedTriangle{triangle, *bounds};
}

std::optional<std::vector<GeneratedTriangle>>
Generator::generate_triangles(std::size_t count) {
    std::vector<GeneratedTriangle> result;
    result.reserve(count);

    for (std::size_t index = 0; index < count; ++index) {
        auto triangle = generate_triangle();

        if (!triangle) {
            return std::nullopt;
        }

        result.push_back(*triangle);
    }

    return result;
}

} // namespace triangles
