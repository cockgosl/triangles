#include <gtest/gtest.h>

#include <limits>
#include <stdexcept>

#include "geometry/triangle.hpp"

using triangles::Triangle3;
using triangles::Vector3;

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

TEST(Triangle3Test, SmallTriangleIsNotDegenerate) {
    Triangle3 triangle(Vector3(0, 0, 0), Vector3(0x1p-100, 0, 0),
                       Vector3(0, 0x1p-100, 0));

    EXPECT_FALSE(triangle.is_degenerate());
}

TEST(Triangle3Test, Degenerate) {
    Triangle3 triangle(Vector3(0.0, 0.0, 0.0), Vector3(1.0, 1.0, 1.0), Vector3(2.0, 2.0, 2.0));

    EXPECT_TRUE(triangle.is_degenerate());
}

TEST(Triangle3Test, CalculatesBounds) {
    const Triangle3 triangle{
        Vector3{1.0, 2.0, 0.0},
        Vector3{4.0, 1.0, 3.0},
        Vector3{2.0, 5.0, 1.0}
    };

    const auto bounds = triangle.bounds();

    ASSERT_TRUE(bounds);

    EXPECT_EQ(bounds->get_min(), Vector3(1.0, 1.0, 0.0));
    EXPECT_EQ(bounds->get_max(), Vector3(4.0, 5.0, 3.0));
}

TEST(Triangle3Test, NonfiniteCoordinatesCannotBeClassified) {
    const Triangle3 triangle(
        Vector3(0, 0, std::numeric_limits<double>::quiet_NaN()),
        Vector3(1, 0, 0), Vector3(0, 1, 0));

    EXPECT_THROW((void)triangle.is_degenerate(), std::invalid_argument);
}
