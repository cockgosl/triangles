#include <gtest/gtest.h>

#include <cmath>
#include <limits>

#include "line3.hpp"
#include "plane3.hpp"

using triangles::Line3;
using triangles::Plane3;
using triangles::Vector3;

constexpr double EPSILON = std::numeric_limits<double>::epsilon();

TEST(Line3Test, NormalizesDirectionAndPreservesSupportPoint) {
    const Vector3 point(1.0, 2.0, 3.0);
    const auto line = Line3::Create(Vector3(3.0, 4.0, 0.0), point);
    const auto through_origin = Line3::Create(Vector3(3.0, 4.0, 0.0));

    ASSERT_TRUE(line.has_value());
    ASSERT_TRUE(through_origin.has_value());
    EXPECT_NEAR(line->get_direction().length(), 1.0, EPSILON);
    EXPECT_TRUE(line->get_point() == point);
    EXPECT_TRUE(through_origin->get_point() == Vector3{});
    EXPECT_TRUE(through_origin->get_direction() == line->get_direction());
}

TEST(Line3Test, RejectsInvalidDirectionOrSupportPoint) {
    EXPECT_FALSE(Line3::Create(Vector3{}).has_value());

    for (const double invalid : {std::numeric_limits<double>::quiet_NaN(),
                                  std::numeric_limits<double>::infinity()}) {
        EXPECT_FALSE(Line3::Create(Vector3(invalid, 0.0, 1.0)).has_value());
        EXPECT_FALSE(Line3::Create(Vector3(1.0, 0.0, 0.0),
                                  Vector3(0.0, invalid, 0.0)).has_value());
    }
}

TEST(Line3Test, EqualityRecognizesSameGeometricLine) {
    const auto first = Line3::Create(Vector3(2.0, 0.0, 0.0), Vector3(1.0, 2.0, 3.0));
    const auto same = Line3::Create(Vector3(-4.0, 0.0, 0.0), Vector3(5.0, 2.0, 3.0));
    const auto offset = Line3::Create(Vector3(1.0, 0.0, 0.0), Vector3(1.0, 3.0, 3.0));
    const auto crossing = Line3::Create(Vector3(0.0, 1.0, 0.0), Vector3(1.0, 2.0, 3.0));

    ASSERT_TRUE(first.has_value());
    ASSERT_TRUE(same.has_value());
    ASSERT_TRUE(offset.has_value());
    ASSERT_TRUE(crossing.has_value());
    EXPECT_TRUE(*first == *same);
    EXPECT_FALSE(*first == *offset);
    EXPECT_FALSE(*first == *crossing);
}

TEST(Line3Test, PlaneFactoryHandlesCrossingParallelAndCoincidentPlanes) {
    const auto first = Plane3::Create(Vector3(2.0, 0.0, 0.0), Vector3(1.0, 0.0, 0.0));
    const auto crossing = Plane3::Create(Vector3(0.0, 0.0, 1.0), Vector3(0.0, 0.0, 1.0));
    const auto parallel = Plane3::Create(Vector3(3.0, 0.0, 0.0), Vector3(1.0, 0.0, 0.0));
    const auto coincident = Plane3::Create(Vector3(2.0, 4.0, 0.0), Vector3(-1.0, 0.0, 0.0));

    ASSERT_TRUE(first.has_value());
    ASSERT_TRUE(crossing.has_value());
    ASSERT_TRUE(parallel.has_value());
    ASSERT_TRUE(coincident.has_value());

    const auto line = Line3::Create(*first, *crossing);
    ASSERT_TRUE(line.has_value());
    EXPECT_NEAR(line->get_point().x, 2.0, EPSILON);
    EXPECT_NEAR(line->get_point().z, 1.0, EPSILON);
    EXPECT_NEAR(line->get_direction().x, 0.0, EPSILON);
    EXPECT_NEAR(std::fabs(line->get_direction().y), 1.0, EPSILON);
    EXPECT_NEAR(line->get_direction().z, 0.0, EPSILON);
    EXPECT_FALSE(Line3::Create(*first, *parallel).has_value());
    EXPECT_FALSE(Line3::Create(*first, *coincident).has_value());
}
