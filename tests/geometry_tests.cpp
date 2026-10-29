#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <limits>

#include "geometry.hpp"

using triangles::Triangle3;
using triangles::Vector3;

TEST(TriangleIntersectionTest, NonCoplanarCutsOverlap) {
    const Triangle3 first(Vector3(-2, -1, 0), Vector3(2, -1, 0), Vector3(0, 1, 0));
    const Triangle3 second(Vector3(-2, 0, -1), Vector3(2, 0, -1), Vector3(0, 0, 1));

    // Both triangles cut the x axis in [-1, 1].
    const auto result = triangles::intersection(first, second);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result.value());
}

TEST(TriangleIntersectionTest, BothStraddleButCutsAreDisjoint) {
    const Triangle3 first(Vector3(-2, -1, 0), Vector3(2, -1, 0), Vector3(0, 1, 0));
    const Triangle3 second(Vector3(0.5, 0, -1), Vector3(4.5, 0, -1), Vector3(2.5, 0, 1));

    // Their bounding boxes overlap, but their cuts are [-1, 1] and [1.5, 3.5].
    const auto result = triangles::intersection(first, second);
    ASSERT_TRUE(result.has_value());
    EXPECT_FALSE(result.value());
}

TEST(TriangleIntersectionTest, ObliqueTranslatedPlanesPreserveIntersection) {
    const auto transform = [](const Vector3& point) {
        return Vector3(point.x + point.y + 2 * point.z + 3,
                       2 * point.x - point.y + point.z - 2,
                       point.x + 2 * point.y - point.z + 5);
    };
    const auto transform_triangle = [&](const Triangle3& triangle) {
        return Triangle3(transform(triangle.a), transform(triangle.b), transform(triangle.c));
    };
    const Triangle3 first(Vector3(-2, -1, 0), Vector3(2, -1, 0), Vector3(0, 1, 0));
    const Triangle3 crossing(Vector3(-2, 0, -1), Vector3(2, 0, -1), Vector3(0, 0, 1));
    const Triangle3 separate(Vector3(0.5, 0, -1), Vector3(4.5, 0, -1), Vector3(2.5, 0, 1));

    const auto overlap = triangles::intersection(transform_triangle(first), transform_triangle(crossing));
    ASSERT_TRUE(overlap.has_value());
    EXPECT_TRUE(overlap.value());

    const auto disjoint = triangles::intersection(transform_triangle(first), transform_triangle(separate));
    ASSERT_TRUE(disjoint.has_value());
    EXPECT_FALSE(disjoint.value());
}

TEST(TriangleIntersectionTest, DistinctParallelPlanesDoNotIntersect) {
    const Triangle3 first(Vector3(0, 0, 0), Vector3(2, 0, 0), Vector3(0, 2, 0));
    const Triangle3 second(Vector3(0, 0, 1), Vector3(2, 0, 1), Vector3(0, 2, 1));

    const auto result = triangles::intersection(first, second);
    ASSERT_TRUE(result.has_value());
    EXPECT_FALSE(result.value());
}

TEST(TriangleIntersectionTest, NonCoplanarCutsTouchAtEndpoint) {
    const Triangle3 first(Vector3(-2, -1, 0), Vector3(2, -1, 0), Vector3(0, 1, 0));
    const Triangle3 second(Vector3(0, 0, -1), Vector3(4, 0, -1), Vector3(2, 0, 1));

    // The cuts [-1, 1] and [1, 3] share exactly (1, 0, 0).
    const auto result = triangles::intersection(first, second);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result.value());
}

TEST(TriangleIntersectionTest, VertexTouchesInteriorOfOtherTriangle) {
    const Triangle3 first(Vector3(-2, -1, 0), Vector3(2, -1, 0), Vector3(0, 1, 0));
    const Triangle3 second(Vector3(0, 0, 0), Vector3(0, -1, 1), Vector3(0, 1, 1));

    const auto result = triangles::intersection(first, second);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result.value());
}

TEST(TriangleIntersectionTest, VertexOnOtherPlaneCanMissOtherTriangle) {
    const Triangle3 first(Vector3(-2, -1, 0), Vector3(2, -1, 0), Vector3(0, 1, 0));
    const Triangle3 second(Vector3(0, 2, 0), Vector3(0, 0.5, 1), Vector3(0, 3, 1));

    // Only (0, 2, 0) lies on z = 0, outside the first triangle.
    const auto result = triangles::intersection(first, second);
    ASSERT_TRUE(result.has_value());
    EXPECT_FALSE(result.value());
}

TEST(TriangleIntersectionTest, EdgeOnOtherPlaneOverlapsOtherTriangle) {
    const Triangle3 first(Vector3(-2, -1, 0), Vector3(2, -1, 0), Vector3(0, 1, 0));
    const Triangle3 second(Vector3(-0.5, 0, 0), Vector3(0.5, 0, 0), Vector3(0, 0, 1));

    const auto result = triangles::intersection(first, second);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result.value());
}

TEST(TriangleIntersectionTest, EdgeOnOtherPlaneCanMissOtherTriangle) {
    const Triangle3 first(Vector3(-2, -1, 0), Vector3(2, -1, 0), Vector3(0, 1, 0));
    const Triangle3 second(Vector3(1.5, 0, 0), Vector3(2, 0, 0), Vector3(0, 0, 1));

    // The cuts [-1, 1] and [1.5, 2] are disjoint despite overlapping boxes.
    const auto result = triangles::intersection(first, second);
    ASSERT_TRUE(result.has_value());
    EXPECT_FALSE(result.value());
}

TEST(TriangleIntersectionTest, ArgumentOrderAndVertexOrderPreserveResult) {
    const Triangle3 first(Vector3(-2, -1, 0), Vector3(2, -1, 0), Vector3(0, 1, 0));
    const std::array<Triangle3, 2> seconds{
        Triangle3(Vector3(-2, 0, -1), Vector3(2, 0, -1), Vector3(0, 0, 1)),
        Triangle3(Vector3(0.5, 0, -1), Vector3(4.5, 0, -1), Vector3(2.5, 0, 1))
    };

    const auto permutations = [](const Triangle3& triangle) {
        return std::array<Triangle3, 6>{
            Triangle3(triangle.a, triangle.b, triangle.c),
            Triangle3(triangle.b, triangle.c, triangle.a),
            Triangle3(triangle.c, triangle.a, triangle.b),
            Triangle3(triangle.a, triangle.c, triangle.b),
            Triangle3(triangle.c, triangle.b, triangle.a),
            Triangle3(triangle.b, triangle.a, triangle.c)
        };
    };

    for (std::size_t case_index = 0; case_index < seconds.size(); ++case_index) {
        SCOPED_TRACE(case_index);
        const bool expected = case_index == 0;
        for (const auto& first_order : permutations(first)) {
            for (const auto& second_order : permutations(seconds[case_index])) {
                const auto forward = triangles::intersection(first_order, second_order);
                ASSERT_TRUE(forward.has_value());
                EXPECT_EQ(forward.value(), expected);

                const auto backward = triangles::intersection(second_order, first_order);
                ASSERT_TRUE(backward.has_value());
                EXPECT_EQ(backward.value(), expected);
            }
        }
    }
}

TEST(TriangleIntersectionTest, CoplanarTriangleContainedInOtherTriangle) {
    const Triangle3 outer(Vector3(0, 0, 0), Vector3(6, 0, 0), Vector3(0, 6, 0));
    const Triangle3 inner(Vector3(1, 1, 0), Vector3(2, 1, 0), Vector3(1, 2, 0));

    const auto result = triangles::intersection(outer, inner);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result.value());
}

TEST(TriangleIntersectionTest, CoplanarEdgesCrossWithoutContainedVertices) {
    const Triangle3 first(Vector3(-2, -1, 0), Vector3(2, -1, 0), Vector3(0, 2, 0));
    const Triangle3 second(Vector3(-2, 1, 0), Vector3(2, 1, 0), Vector3(0, -2, 0));

    const auto result = triangles::intersection(first, second);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result.value());
}

TEST(TriangleIntersectionTest, VerticalCoplanarTrianglesSeparatedAlongZ) {
    const Triangle3 first(Vector3(0, 0, 0), Vector3(0, 2, 0), Vector3(0, 0, 2));
    const Triangle3 second(Vector3(0, 0, 3), Vector3(0, 2, 3), Vector3(0, 0, 5));

    // Identical x/y bounds must not hide the disjoint z ranges [0, 2] and [3, 5].
    const auto result = triangles::intersection(first, second);
    ASSERT_TRUE(result.has_value());
    EXPECT_FALSE(result.value());
}

TEST(TriangleIntersectionTest, RejectsNaNCoordinateInEitherArgument) {
    const Triangle3 valid(Vector3(0, 0, 0), Vector3(2, 0, 0), Vector3(0, 2, 0));
    const Triangle3 invalid(Vector3(std::numeric_limits<double>::quiet_NaN(), 0, 0),
                            Vector3(2, 0, 0), Vector3(0, 2, 0));

    EXPECT_FALSE(triangles::intersection(invalid, valid).has_value());
    EXPECT_FALSE(triangles::intersection(valid, invalid).has_value());
}

TEST(TriangleIntersectionTest, RejectsInfiniteCoordinateInEitherArgument) {
    const Triangle3 valid(Vector3(0, 0, 0), Vector3(2, 0, 0), Vector3(0, 2, 0));
    const Triangle3 invalid(Vector3(0, 0, 0), Vector3(2, 0, 0),
                            Vector3(0, 2, std::numeric_limits<double>::infinity()));

    EXPECT_FALSE(triangles::intersection(invalid, valid).has_value());
    EXPECT_FALSE(triangles::intersection(valid, invalid).has_value());
}

TEST(TriangleIntersectionTest, RejectsDegenerateTriangleInEitherArgument) {
    const Triangle3 valid(Vector3(0, 0, 0), Vector3(2, 0, 0), Vector3(0, 2, 0));
    const Triangle3 invalid(Vector3(0, 0, 0), Vector3(1, 1, 1), Vector3(2, 2, 2));

    EXPECT_FALSE(triangles::intersection(invalid, valid).has_value());
    EXPECT_FALSE(triangles::intersection(valid, invalid).has_value());
}

TEST(TriangleIntersectionTest, ReportsOverflowOfPlaneValue) {
    const double large = 1.4e308;
    const double next = std::nextafter(large, std::numeric_limits<double>::infinity());
    const Triangle3 first(Vector3(0, 0, 0), Vector3(1, -1, 0), Vector3(0, 0, 1));
    const Triangle3 second(Vector3(large, large, 0), Vector3(next, next, 0),
                           Vector3(large, large, 1e-292));

    ASSERT_TRUE(std::isfinite(first.area()));
    ASSERT_TRUE(std::isfinite(second.area()));
    EXPECT_FALSE(triangles::intersection(first, second).has_value());
    EXPECT_FALSE(triangles::intersection(second, first).has_value());
}
