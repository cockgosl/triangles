#include <gtest/gtest.h>

#include <array>
#include <limits>

#include "plane3.hpp"

using triangles::Plane3;
using triangles::Triangle3;
using triangles::Vector3;

constexpr double EPSILON = std::numeric_limits<double>::epsilon();

TEST(Plane3Test, PointAndNormalDefineSupportingPlane) {
    const Vector3 point(0.0, 0.0, 3.0);
    const auto plane = Plane3::Create(point, Vector3(0.0, 0.0, 4.0));

    ASSERT_TRUE(plane.has_value());
    EXPECT_NEAR(plane->get_normal().length(), 1.0, EPSILON);
    EXPECT_NEAR(plane->get_normal().dot(point) + plane->get_d(), 0.0, EPSILON);
    EXPECT_NEAR(plane->get_normal().dot(Vector3(2.0, -1.0, 3.0)) + plane->get_d(),
                0.0, EPSILON);
    EXPECT_NE(plane->get_normal().dot(Vector3(0.0, 0.0, 4.0)) + plane->get_d(), 0.0);
}

TEST(Plane3Test, TriangleAndThreePointsDefineSameSupportingPlane) {
    const Triangle3 triangle(Vector3(1.0, 0.0, 2.0),
                             Vector3(3.0, 0.0, 2.0),
                             Vector3(1.0, 4.0, 2.0));
    const std::array planes{
        Plane3::Create(triangle),
        Plane3::Create(triangle.a, triangle.b, triangle.c)
    };

    for (const auto& plane : planes) {
        ASSERT_TRUE(plane.has_value());
        EXPECT_NEAR(plane->get_normal().length(), 1.0, EPSILON);
        for (const auto& point : {triangle.a, triangle.b, triangle.c})
            EXPECT_NEAR(plane->get_normal().dot(point) + plane->get_d(), 0.0, EPSILON);
    }
}

TEST(Plane3Test, RejectsDegenerateAndNonFiniteInputs) {
    const Vector3 first(0.0, 0.0, 0.0);
    const Vector3 second(1.0, 1.0, 1.0);
    const Vector3 third(2.0, 2.0, 2.0);

    EXPECT_FALSE(Plane3::Create(first, Vector3{}).has_value());
    EXPECT_FALSE(Plane3::Create(first, second, third).has_value());
    EXPECT_FALSE(Plane3::Create(Triangle3(first, second, third)).has_value());

    for (const double invalid : {std::numeric_limits<double>::quiet_NaN(),
                                  std::numeric_limits<double>::infinity()}) {
        EXPECT_FALSE(Plane3::Create(Vector3(invalid, 0.0, 0.0),
                                   Vector3(0.0, 0.0, 1.0)).has_value());
        EXPECT_FALSE(Plane3::Create(first, Vector3(0.0, invalid, 1.0)).has_value());
    }
}
