#include "geometry/robust_predicates.hpp"

#include <gtest/gtest.h>

#include <stdexcept>

using namespace triangles::robust;

TEST(RobustArithmeticTest,
     AdditionAndSubtractionRetainResiduals) {
    const auto sum = two_sum(0x1p54, 3.0);
    EXPECT_EQ(sum.value, 0x1p54 + 4.0);
    EXPECT_EQ(sum.error, -1.0);

    const auto difference = two_diff(0x1p54, 3.0);
    EXPECT_EQ(difference.value, 0x1p54 - 4.0);
    EXPECT_EQ(difference.error, 1.0);
}

TEST(RobustArithmeticTest,
     FastTwoSumRequiresOrderedMagnitudes) {
    EXPECT_THROW((void)fast_two_sum(1.0, 0x1p54), std::invalid_argument);
}

TEST(RobustArithmeticTest,
     ProductRetainsRoundedResidual) {
    const auto product = two_product(1.0 + 0x1p-27, 1.0 - 0x1p-27);
    EXPECT_EQ(product.value, 1.0);
    EXPECT_EQ(product.error, -0x1p-54);
}

TEST(RobustPredicatesTest,
     Orient2dResolvesProductCancellation) {
    EXPECT_EQ(orient2d({1.0 + 0x1p-27, 1.0}, {1.0, 1.0 - 0x1p-27}, {0.0, 0.0}), Sign::negative);
}

TEST(RobustPredicatesTest,
     Orient3dRetainsCoordinateDifferenceTails) {
    EXPECT_EQ(orient3d({0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0x1p54, 0x1p54, 0x1p54}), Sign::negative);
}
