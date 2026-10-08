#include <gtest/gtest.h>

#include "triangle.hpp"
constexpr double EPSILON = std::numeric_limits<double>::epsilon();


TEST(Triangle3Test, Edges) {
    Triangle3 triangle(Vector3(0.0, 0.0, 0.0), Vector3(1.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0));

    Vector3 ab = triangle.edge_AB();
    Vector3 ac = triangle.edge_AC();

    EXPECT_DOUBLE_EQ(ab.x, 1.0);
    EXPECT_DOUBLE_EQ(ab.y, 0.0);
    EXPECT_DOUBLE_EQ(ab.z, 0.0);

    EXPECT_DOUBLE_EQ(ac.x, 0.0);
    EXPECT_DOUBLE_EQ(ac.y, 1.0);
    EXPECT_DOUBLE_EQ(ac.z, 0.0);
}

TEST(Triangle3Test, Area) {
    Triangle3 triangle(Vector3(0.0, 0.0, 0.0), Vector3(1.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0));

    EXPECT_NEAR(triangle.area(), 0.5, EPSILON);
}

TEST(Triangle3Test, Normal) {
    Triangle3 triangle(Vector3(0.0, 0.0, 0.0), Vector3(1.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0));

    Vector3 normal = triangle.normal();

    EXPECT_NEAR(normal.x, 0.0, EPSILON);
    EXPECT_NEAR(normal.y, 0.0, EPSILON);
    EXPECT_NEAR(normal.z, 1.0, EPSILON);
}

TEST(Triangle3Test, NonDegenerate) {
    Triangle3 triangle(Vector3(0.0, 0.0, 0.0), Vector3(1.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0));

    EXPECT_FALSE(triangle.is_degenerate());
}

TEST(Triangle3Test, Degenerate) {
    Triangle3 triangle(Vector3(0.0, 0.0, 0.0), Vector3(1.0, 1.0, 1.0), Vector3(2.0, 2.0, 2.0));

    EXPECT_TRUE(triangle.is_degenerate());
}
