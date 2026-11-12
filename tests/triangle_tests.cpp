#include <gtest/gtest.h>

#include <limits>
#include <sstream>
#include <string>
#include <stdexcept>

#include "geometry/triangle.hpp"

using triangles::Triangle3;
using triangles::Vector3;

constexpr double EPSILON = std::numeric_limits<double>::epsilon();


TEST(Triangle3Test,
     Edges) {
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

TEST(Triangle3Test,
     Area) {
    Triangle3 triangle(Vector3(0.0, 0.0, 0.0), Vector3(1.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0));

    EXPECT_NEAR(triangle.area(), 0.5, EPSILON);
}

TEST(Triangle3Test,
     Normal) {
    Triangle3 triangle(Vector3(0.0, 0.0, 0.0), Vector3(1.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0));

    Vector3 normal = triangle.normal();

    EXPECT_NEAR(normal.x, 0.0, EPSILON);
    EXPECT_NEAR(normal.y, 0.0, EPSILON);
    EXPECT_NEAR(normal.z, 1.0, EPSILON);
}

TEST(Triangle3Test,
     SmallTriangleIsNotDegenerate) {
    Triangle3 triangle(Vector3(0, 0, 0), Vector3(0x1p-100, 0, 0), Vector3(0, 0x1p-100, 0));

    EXPECT_FALSE(triangle.is_degenerate());
}

TEST(Triangle3Test,
     Degenerate) {
    Triangle3 triangle(Vector3(0.0, 0.0, 0.0), Vector3(1.0, 1.0, 1.0), Vector3(2.0, 2.0, 2.0));

    EXPECT_TRUE(triangle.is_degenerate());
}

TEST(Triangle3Test,
     CalculatesBounds) {
    const Triangle3 triangle{Vector3{1.0, 2.0, 0.0}, Vector3{4.0, 1.0, 3.0}, Vector3{2.0, 5.0, 1.0}};

    const auto bounds = triangle.bounds();

    ASSERT_TRUE(bounds);

    EXPECT_EQ(bounds->get_min(), Vector3(1.0, 1.0, 0.0));
    EXPECT_EQ(bounds->get_max(), Vector3(4.0, 5.0, 3.0));
}

TEST(Triangle3Test,
     NonfiniteCoordinatesCannotBeClassified) {
    const Triangle3 triangle(Vector3(0, 0, std::numeric_limits<double>::quiet_NaN()), Vector3(1, 0, 0), Vector3(0, 1, 0));

    EXPECT_THROW((void)triangle.is_degenerate(), std::invalid_argument);
}

TEST(TriangleInputTest,
     ReadsCountWithinStrictBounds) {
    for (const auto* token : {"1", "999999"}) {
        SCOPED_TRACE(token);
        std::istringstream input{token};
        std::string error = "previous error";
        const auto count = triangles::read_triangle_count(input, &error);
        ASSERT_TRUE(count);
        EXPECT_EQ(*count, std::stoull(token));
        EXPECT_TRUE(error.empty());
    }

    for (const auto* token : {"", "0", "-1", "1000000", "1.5", "184467440737095516160"}) {
        SCOPED_TRACE(token);
        std::istringstream input{token};
        std::string error;
        EXPECT_FALSE(triangles::read_triangle_count(input, &error));
        EXPECT_FALSE(error.empty());
    }
}

TEST(TriangleInputTest,
     ReadsVerticesAndBoundsAtLargeOffset) {
    std::istringstream input{"1000000000000 +2 0  1000000000001 2 0  1000000000000 3 0\n"
                             "1000000000002 4 0  1000000000003 4 0  1000000000002 5 0"};
    std::string error = "previous error";
    const auto result = triangles::read_triangles(input, 2, &error);

    ASSERT_TRUE(result) << error;
    ASSERT_EQ(result->size(), 2u);
    EXPECT_TRUE(error.empty());
    EXPECT_EQ((*result)[0].triangle.a, Vector3(1000000000000, 2, 0));
    EXPECT_EQ((*result)[0].triangle.b, Vector3(1000000000001, 2, 0));
    EXPECT_EQ((*result)[0].triangle.c, Vector3(1000000000000, 3, 0));
    EXPECT_EQ((*result)[0].bounds.get_min(), Vector3(1000000000000, 2, 0));
    EXPECT_EQ((*result)[0].bounds.get_max(), Vector3(1000000000001, 3, 0));
    EXPECT_EQ((*result)[1].triangle.c, Vector3(1000000000002, 5, 0));
}

TEST(TriangleInputTest,
     RejectsInvalidCountBeforeReadingOrAllocating) {
    for (const std::size_t count : {std::size_t{0}, std::size_t{1000000}}) {
        std::istringstream input{"untouched"};
        std::string error;
        EXPECT_FALSE(triangles::read_triangles(input, count, &error));
        EXPECT_FALSE(error.empty());
        EXPECT_EQ(input.peek(), 'u');
    }
}

TEST(TriangleInputTest,
     RejectsIncompleteOrMalformedTriangleWithoutPartialResult) {
    for (const auto* tail : {"0 0 0  1 0 0  0 1", "0 0 0  1 word 0  0 1 0", "0 0 0  1 0 0  0 1 0tail"}) {
        SCOPED_TRACE(tail);
        std::istringstream input{std::string{"0 0 0  1 0 0  0 1 0\n"} + tail};
        std::string error;
        EXPECT_FALSE(triangles::read_triangles(input, 2, &error));
        EXPECT_NE(error.find("Triangle 2:"), std::string::npos);
    }
}

TEST(TriangleInputTest,
     RejectsDegenerateTriangle) {
    std::istringstream input{"0 0 0  1 1 1  2 2 2"};
    std::string error;
    EXPECT_FALSE(triangles::read_triangles(input, 1, &error));
    EXPECT_NE(error.find("collinear"), std::string::npos);
}

TEST(TriangleInputTest,
     RejectsCoordinatesUnsupportedByPredicates) {
    for (const auto* coordinate : {"nan", "inf", "1e100", "1e-100", "1e-999"}) {
        SCOPED_TRACE(coordinate);
        std::istringstream input{std::string{coordinate} + " 0 0  1 0 0  0 1 0"};
        std::string error;
        EXPECT_FALSE(triangles::read_triangles(input, 1, &error));
        EXPECT_NE(error.find("Triangle 1:"), std::string::npos);
    }
}
