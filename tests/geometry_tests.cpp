#include <gtest/gtest.h>
#include "geometry.hpp"

TEST(Vector3Test, Constructor) {
    Vector3 vector(1.0, 2.0, 3.0);

    EXPECT_DOUBLE_EQ(vector.x, 1.0);
    EXPECT_DOUBLE_EQ(vector.y, 2.0);
    EXPECT_DOUBLE_EQ(vector.z, 3.0);
}

TEST(Vector3Test, DefaultConstructor) {
    Vector3 vector;

    EXPECT_DOUBLE_EQ(vector.x, 0.0);
    EXPECT_DOUBLE_EQ(vector.y, 0.0);
    EXPECT_DOUBLE_EQ(vector.z, 0.0);
}

TEST(Vector3Test, Addition) {
    Vector3 a(1.0, 2.0, 3.0);
    Vector3 b(4.0, 5.0, 6.0);

    Vector3 result = a + b;

    EXPECT_DOUBLE_EQ(result.x, 5.0);
    EXPECT_DOUBLE_EQ(result.y, 7.0);
    EXPECT_DOUBLE_EQ(result.z, 9.0);
}

TEST(Vector3Test, Subtraction) {
    Vector3 a(4.0, 5.0, 6.0);
    Vector3 b(1.0, 2.0, 3.0);

    Vector3 result = a - b;

    EXPECT_DOUBLE_EQ(result.x, 3.0);
    EXPECT_DOUBLE_EQ(result.y, 3.0);
    EXPECT_DOUBLE_EQ(result.z, 3.0);
}

TEST(Vector3Test, ScalarMultiplication) {
    Vector3 vector(1.0, 2.0, 3.0);

    Vector3 result = vector * 2.0;

    EXPECT_DOUBLE_EQ(result.x, 2.0);
    EXPECT_DOUBLE_EQ(result.y, 4.0);
    EXPECT_DOUBLE_EQ(result.z, 6.0);
}

TEST(Vector3Test, Length) {
    constexpr double epsilon = std::numeric_limits<double>::epsilon();
    Vector3 vector(3.0, 4.0, 0.0);

    EXPECT_NEAR(vector.length(), 5.0, epsilon);
}

TEST(Vector3Test, Normalized) {
    constexpr double epsilon = std::numeric_limits<double>::epsilon();
    Vector3 vector(3.0, 4.0, 0.0);

    Vector3 result = vector.normalized();

    EXPECT_NEAR(result.x, 0.6, epsilon);
    EXPECT_NEAR(result.y, 0.8, epsilon);
    EXPECT_NEAR(result.z, 0.0, epsilon);
    EXPECT_NEAR(result.length(), 1.0, epsilon);
}

TEST(Vector3Test, DotProduct) {
    Vector3 a(1.0, 2.0, 3.0);
    Vector3 b(4.0, 5.0, 6.0);

    EXPECT_DOUBLE_EQ(a.dot(b), 32.0);
}

TEST(Vector3Test, CrossProduct) {
    Vector3 a(1.0, 0.0, 0.0);
    Vector3 b(0.0, 1.0, 0.0);

    Vector3 result = a.cross(b);

    EXPECT_DOUBLE_EQ(result.x, 0.0);
    EXPECT_DOUBLE_EQ(result.y, 0.0);
    EXPECT_DOUBLE_EQ(result.z, 1.0);
}

TEST(Point3Test, Constructor) {
    Point3 point(1.0, 2.0, 3.0);

    EXPECT_DOUBLE_EQ(point.x, 1.0);
    EXPECT_DOUBLE_EQ(point.y, 2.0);
    EXPECT_DOUBLE_EQ(point.z, 3.0);
}

TEST(Point3Test, PointMinusPoint) {
    Point3 a(4.0, 5.0, 6.0);
    Point3 b(1.0, 2.0, 3.0);

    Vector3 result = a - b;

    EXPECT_DOUBLE_EQ(result.x, 3.0);
    EXPECT_DOUBLE_EQ(result.y, 3.0);
    EXPECT_DOUBLE_EQ(result.z, 3.0);
}

TEST(Point3Test, PointPlusVector) {
    Point3 point(1.0, 2.0, 3.0);
    Vector3 vector(4.0, 5.0, 6.0);

    Point3 result = point + vector;

    EXPECT_DOUBLE_EQ(result.x, 5.0);
    EXPECT_DOUBLE_EQ(result.y, 7.0);
    EXPECT_DOUBLE_EQ(result.z, 9.0);
}

TEST(Point3Test, PointMinusVector) {
    Point3 point(4.0, 5.0, 6.0);
    Vector3 vector(1.0, 2.0, 3.0);

    Point3 result = point - vector;

    EXPECT_DOUBLE_EQ(result.x, 3.0);
    EXPECT_DOUBLE_EQ(result.y, 3.0);
    EXPECT_DOUBLE_EQ(result.z, 3.0);
}

TEST(Triangle3Test, Edges) {
    Triangle3 triangle(Point3(0.0, 0.0, 0.0), Point3(1.0, 0.0, 0.0), Point3(0.0, 1.0, 0.0));

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
    constexpr double epsilon = std::numeric_limits<double>::epsilon();
    Triangle3 triangle(Point3(0.0, 0.0, 0.0), Point3(1.0, 0.0, 0.0), Point3(0.0, 1.0, 0.0));

    EXPECT_NEAR(triangle.area(), 0.5, epsilon);
}

TEST(Triangle3Test, Normal) {
    constexpr double epsilon = std::numeric_limits<double>::epsilon();
    Triangle3 triangle(Point3(0.0, 0.0, 0.0), Point3(1.0, 0.0, 0.0), Point3(0.0, 1.0, 0.0));

    Vector3 normal = triangle.normal();

    EXPECT_NEAR(normal.x, 0.0, epsilon);
    EXPECT_NEAR(normal.y, 0.0, epsilon);
    EXPECT_NEAR(normal.z, 1.0, epsilon);
}

TEST(Triangle3Test, NonDegenerate) {
    Triangle3 triangle(Point3(0.0, 0.0, 0.0), Point3(1.0, 0.0, 0.0), Point3(0.0, 1.0, 0.0));

    EXPECT_FALSE(triangle.is_degenerate());
}

TEST(Triangle3Test, Degenerate) {
    Triangle3 triangle(Point3(0.0, 0.0, 0.0), Point3(1.0, 1.0, 1.0), Point3(2.0, 2.0, 2.0));

    EXPECT_TRUE(triangle.is_degenerate());
}
