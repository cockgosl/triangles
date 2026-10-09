#include <gtest/gtest.h>

#include <limits>

#include "triangle.hpp"

using triangles::Triangle3;
using triangles::Vector3;

constexpr double EPSILON = std::numeric_limits<double>::epsilon();


TEST(Triangle3Test, Edges) {
    Triangle3 triangle(Vector3(0.0, 0.0, 0.0), Vector3(1.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0));

    Vector3 ab = triangle.edge_AB();
    Vector3 ac = triangle.edge_AC();

    EXPECT_DOUBLE_EQ(ab.get_x(), 1.0);
    EXPECT_DOUBLE_EQ(ab.get_y(), 0.0);
    EXPECT_DOUBLE_EQ(ab.get_z(), 0.0);

    EXPECT_DOUBLE_EQ(ac.get_x(), 0.0);
    EXPECT_DOUBLE_EQ(ac.get_y(), 1.0);
    EXPECT_DOUBLE_EQ(ac.get_z(), 0.0);
}

TEST(Triangle3Test, Area) {
    Triangle3 triangle(Vector3(0.0, 0.0, 0.0), Vector3(1.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0));

    EXPECT_NEAR(triangle.area(), 0.5, EPSILON);
}

TEST(Triangle3Test, Normal) {
    Triangle3 triangle(Vector3(0.0, 0.0, 0.0), Vector3(1.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0));

    Vector3 normal = triangle.normal();

    EXPECT_NEAR(normal.get_x(), 0.0, EPSILON);
    EXPECT_NEAR(normal.get_y(), 0.0, EPSILON);
    EXPECT_NEAR(normal.get_z(), 1.0, EPSILON);
}

TEST(Triangle3Test, NonDegenerate) {
    Triangle3 triangle(Vector3(0.0, 0.0, 0.0), Vector3(1.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0));

    EXPECT_FALSE(triangle.is_degenerate());
}

TEST(Triangle3Test, Degenerate) {
    Triangle3 triangle(Vector3(0.0, 0.0, 0.0), Vector3(1.0, 1.0, 1.0), Vector3(2.0, 2.0, 2.0));

    EXPECT_TRUE(triangle.is_degenerate());
}
