#include <gtest/gtest.h>

#include "generator.hpp"

#include <cmath>
#include <limits>

using triangles::Generator;

TEST(GeneratorTests, ReturnsRequestedCount) {
    auto generator = Generator::Create();

    ASSERT_TRUE(generator.has_value());

    auto triangles = generator->generate_triangles(10);

    ASSERT_TRUE(triangles.has_value());
    EXPECT_EQ(triangles->size(), 10u);

    auto empty_result = generator->generate_triangles(0);

    ASSERT_TRUE(empty_result.has_value());
    EXPECT_TRUE(empty_result->empty());
}

TEST(GeneratorTests, RejectsInvalidParameters) {
    EXPECT_FALSE(Generator::Create(-1.0).has_value());
    EXPECT_FALSE(Generator::Create(0.0).has_value());
    EXPECT_FALSE(Generator::Create(1001.0).has_value());

    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double infinity = std::numeric_limits<double>::infinity();

    EXPECT_FALSE(Generator::Create(nan).has_value());
    EXPECT_FALSE(Generator::Create(infinity).has_value());

    EXPECT_FALSE(Generator::Create(10.0, nan).has_value());
    EXPECT_FALSE(Generator::Create(10.0, 1.0, infinity).has_value());
}

TEST(GeneratorTests, GeneratedVectorHasRequestedLength) {
    auto generator = Generator::Create(10.0, 1.0, 0.5);

    ASSERT_TRUE(generator.has_value());

    constexpr double kLength = 3.0;
    constexpr int kSamples = 100;

    for (int i = 0; i < kSamples; ++i) {
        auto vector = generator->generate_vector(kLength);

        ASSERT_TRUE(vector.has_value());
        EXPECT_TRUE(std::isfinite(vector->length()));
        EXPECT_NEAR(vector->length(), kLength, 1e-12);
    }
}

TEST(GeneratorTests, GeneratedDirectionsAreApproximatelyUniform) {
    auto generator = Generator::Create(10.0, 1.0, 0.5);

    ASSERT_TRUE(generator.has_value());

    constexpr double kLength = 3.0;
    constexpr int kSamples = 20000;

    double sum_x = 0.0;
    double sum_y = 0.0;
    double sum_z = 0.0;

    double sum_x_squared = 0.0;
    double sum_y_squared = 0.0;
    double sum_z_squared = 0.0;

    for (int i = 0; i < kSamples; ++i) {
        auto vector = generator->generate_vector(kLength);

        ASSERT_TRUE(vector.has_value());

        sum_x += vector->x;
        sum_y += vector->y;
        sum_z += vector->z;

        sum_x_squared += vector->x * vector->x;
        sum_y_squared += vector->y * vector->y;
        sum_z_squared += vector->z * vector->z;
    }

    const double mean_x = sum_x / kSamples;
    const double mean_y = sum_y / kSamples;
    const double mean_z = sum_z / kSamples;

    const double expected_squared_coordinate =
        kLength * kLength / 3.0;

    EXPECT_NEAR(mean_x, 0.0, 0.08);
    EXPECT_NEAR(mean_y, 0.0, 0.08);
    EXPECT_NEAR(mean_z, 0.0, 0.08);

    EXPECT_NEAR(
        sum_x_squared / kSamples, expected_squared_coordinate, 0.12);
    EXPECT_NEAR(
        sum_y_squared / kSamples, expected_squared_coordinate, 0.12);
    EXPECT_NEAR(
        sum_z_squared / kSamples, expected_squared_coordinate, 0.12);
}
