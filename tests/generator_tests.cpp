#include <gtest/gtest.h>

#include "geometry/generator.hpp"

#include <algorithm>

using triangles::AABB;
using triangles::Generator;
using triangles::GeneratorConfig;
using triangles::Vector3;

namespace {

AABB MakeDomain() {
    const auto domain = AABB::Create(
        Vector3{-5.0, -5.0, -5.0},
        Vector3{5.0, 5.0, 5.0}
    );

    return *domain;
}

GeneratorConfig MakeConfig() {
    return GeneratorConfig{
        .domain = MakeDomain(),
        .target_max_edge_length = 1.0,
        .relative_size_variation = 0.1,
        .min_shape_quality = 0.1,
        .seed = 42u,
        .max_shape_attempts = 1000
    };
}

} // namespace

TEST(GeneratorTests,
     CreatesWithValidConfig) {
    const auto generator = Generator::Create(MakeConfig());

    EXPECT_TRUE(generator.has_value());
}

TEST(GeneratorTests,
     RejectsInvalidTargetLength) {
    auto config = MakeConfig();
    config.target_max_edge_length = -1.0;

    EXPECT_FALSE(Generator::Create(config));
}

TEST(GeneratorTests,
     RejectsInvalidSizeVariation) {
    auto config = MakeConfig();
    config.relative_size_variation = 1.0;

    EXPECT_FALSE(Generator::Create(config));
}

TEST(GeneratorTests,
     RejectsInvalidShapeQuality) {
    auto config = MakeConfig();
    config.min_shape_quality = 0.0;

    EXPECT_FALSE(Generator::Create(config));
}

TEST(GeneratorTests,
     RejectsZeroAttempts) {
    auto config = MakeConfig();
    config.max_shape_attempts = 0;

    EXPECT_FALSE(Generator::Create(config));
}

TEST(GeneratorTests,
     RejectsDomainTooSmall) {
    auto config = MakeConfig();

    const auto domain = AABB::Create(
        Vector3{0.0, 0.0, 0.0},
        Vector3{0.1, 0.1, 0.1}
    );

    ASSERT_TRUE(domain.has_value());
    config.domain = *domain;

    EXPECT_FALSE(Generator::Create(config));
}

TEST(GeneratorTests,
     GeneratesTrianglesInsideDomain) {
    const auto domain = AABB::Create(Vector3{-5.0, -5.0, -5.0}, Vector3{5.0, 5.0, 5.0});

    ASSERT_TRUE(domain);

    const GeneratorConfig config{.domain = *domain,
                                 .target_max_edge_length = 1.0,
                                 .relative_size_variation = 0.1,
                                 .min_shape_quality = 0.1,
                                 .seed = 42u,
                                 .max_shape_attempts = 1000};

    auto generator = Generator::Create(config);

    ASSERT_TRUE(generator);

    auto generated = generator->generate_triangles(100);

    ASSERT_TRUE(generated);
    ASSERT_EQ(generated->size(), 100u);

    for (const auto& item : *generated) {
        EXPECT_TRUE(domain->contains(item.bounds));

        const auto& triangle = item.triangle;

        const double ab = triangle.edge_AB().length();
        const double ac = triangle.edge_AC().length();
        const double bc = triangle.edge_BC().length();

        const double max_edge = std::max({ab, ac, bc});

        EXPECT_GE(max_edge, 0.9 - 1e-10);
        EXPECT_LE(max_edge, 1.1 + 1e-10);

        const double quality = 2.0 * triangle.area() / (max_edge * max_edge);

        EXPECT_GE(quality, config.min_shape_quality);
    }
}

TEST(GeneratorTests,
     UnsupportedPredicateRangeReturnsGenerationFailure) {
    auto config = MakeConfig();
    config.domain = *AABB::Create(Vector3{-0x1p264, -0x1p264, -0x1p264}, Vector3{0x1p264, 0x1p264, 0x1p264});
    config.target_max_edge_length = 0x1p260;
    auto generator = Generator::Create(config);

    ASSERT_TRUE(generator.has_value());
    EXPECT_FALSE(generator->generate_triangle().has_value());
}
