#include <gtest/gtest.h>

#include <limits>

#include "vector3.hpp"

using triangles::Vector3;

constexpr double EPSILON = std::numeric_limits<double>::epsilon();

TEST(Vector3Test, Constructor) {
    Vector3 vector(1.0, 2.0, 3.0);

    EXPECT_DOUBLE_EQ(vector.get_x(), 1.0);
    EXPECT_DOUBLE_EQ(vector.get_y(), 2.0);
    EXPECT_DOUBLE_EQ(vector.get_z(), 3.0);
}

TEST(Vector3Test, DefaultConstructor) {
    Vector3 vector;

    EXPECT_DOUBLE_EQ(vector.get_x(), 0.0);
    EXPECT_DOUBLE_EQ(vector.get_y(), 0.0);
    EXPECT_DOUBLE_EQ(vector.get_z(), 0.0);
}

TEST(Vector3Test, Addition) {
    Vector3 vector_1(1.0, 2.0, 3.0);
    Vector3 vector_2(4.0, 5.0, 6.0);

    Vector3 result = vector_1 + vector_2;

    EXPECT_DOUBLE_EQ(result.get_x(), 5.0);
    EXPECT_DOUBLE_EQ(result.get_y(), 7.0);
    EXPECT_DOUBLE_EQ(result.get_z(), 9.0);
}

TEST(Vector3Test, Subtraction) {
    Vector3 vector_1(4.0, 5.0, 6.0);
    Vector3 vector_2(1.0, 2.0, 3.0);

    Vector3 result = vector_1 - vector_2;

    EXPECT_DOUBLE_EQ(result.get_x(), 3.0);
    EXPECT_DOUBLE_EQ(result.get_y(), 3.0);
    EXPECT_DOUBLE_EQ(result.get_z(), 3.0);
}

TEST(Vector3Test, ScalarMultiplication) {
    Vector3 vector(1.0, 2.0, 3.0);

    Vector3 result = vector * 2.0;

    EXPECT_DOUBLE_EQ(result.get_x(), 2.0);
    EXPECT_DOUBLE_EQ(result.get_y(), 4.0);
    EXPECT_DOUBLE_EQ(result.get_z(), 6.0);
}

TEST(Vector3Test, Length) {
    Vector3 vector(3.0, 4.0, 0.0);

    EXPECT_NEAR(vector.length(), 5.0, EPSILON);
}

TEST(Vector3Test, Normalized) {
    Vector3 vector(3.0, 4.0, 0.0);

    Vector3 result = vector.normalized();

    EXPECT_NEAR(result.get_x(), 0.6, EPSILON);
    EXPECT_NEAR(result.get_y(), 0.8, EPSILON);
    EXPECT_NEAR(result.get_z(), 0.0, EPSILON);
    EXPECT_NEAR(result.length(), 1.0, EPSILON);
}

TEST(Vector3Test, DotProduct) {
    Vector3 vector_1(1.0, 2.0, 3.0);
    Vector3 vector_2(4.0, 5.0, 6.0);

    EXPECT_DOUBLE_EQ(vector_1.dot(vector_2), 32.0);
}

TEST(Vector3Test, CrossProduct) {
    Vector3 vector_1(1.0, 0.0, 0.0);
    Vector3 vector_2(0.0, 1.0, 0.0);

    Vector3 result = vector_1.cross(vector_2);

    EXPECT_DOUBLE_EQ(result.get_x(), 0.0);
    EXPECT_DOUBLE_EQ(result.get_y(), 0.0);
    EXPECT_DOUBLE_EQ(result.get_z(), 1.0);
}
