#include <gtest/gtest.h>
#include "generator.hpp"

using triangles::Generator;

TEST(GeneratorTests, ReturnsRequestedCount) {
    Generator generator;

    EXPECT_EQ(generator.generate_triangles(10).size(), 10);
    EXPECT_TRUE(generator.generate_triangles(0).empty());
}
