#include <gtest/gtest.h>

#include <limits>

#include "point2.hpp"

using triangles::Point2;
using triangles::Vector3;

TEST(Point2Test, ProjectionPreservesXYCoordinates) {
    const auto point = Point2::Create(-2.0, 3.5);
    const auto projected = Point2::Create(Vector3(-2.0, 3.5, 8.0));

    ASSERT_TRUE(point.has_value());
    ASSERT_TRUE(projected.has_value());
    EXPECT_DOUBLE_EQ(point->get_x(), -2.0);
    EXPECT_DOUBLE_EQ(point->get_y(), 3.5);
    EXPECT_DOUBLE_EQ(projected->get_x(), point->get_x());
    EXPECT_DOUBLE_EQ(projected->get_y(), point->get_y());
}

TEST(Point2Test, RejectsNonFiniteCoordinates) {
    for (const double invalid : {std::numeric_limits<double>::quiet_NaN(),
                                  std::numeric_limits<double>::infinity()}) {
        EXPECT_FALSE(Point2::Create(invalid, 0.0).has_value());
        EXPECT_FALSE(Point2::Create(0.0, invalid).has_value());
        EXPECT_FALSE(Point2::Create(Vector3(invalid, 0.0, 1.0)).has_value());
        EXPECT_FALSE(Point2::Create(Vector3(0.0, invalid, 1.0)).has_value());
    }
}
