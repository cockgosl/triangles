#include "geometry/robust_predicates.hpp"

#include <bit>
#include <cfenv>
#include <cfloat>
#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>

// clang-format off
#if defined(__FAST_MATH__) ||                                                  \
    (defined(__FINITE_MATH_ONLY__) && __FINITE_MATH_ONLY__ > 0)
#error "Robust predicates require compilation without fast-math."
#endif
// clang-format on

namespace triangles::robust {
namespace {

static_assert(std::numeric_limits<double>::is_iec559 && std::numeric_limits<double>::radix == 2 &&
              std::numeric_limits<double>::digits == 53 && std::numeric_limits<double>::max_exponent == 1024 &&
              std::numeric_limits<double>::min_exponent == -1021 && sizeof(double) == sizeof(std::uint64_t),
              "Robust predicates require IEEE 754 binary64.");
static_assert(FLT_EVAL_METHOD == 0,
              "Robust predicates require double-precision evaluation.");

constexpr double splitter = 0x1p27 + 1.0;
constexpr double unit_roundoff = 0x1p-53;
constexpr double orient2d_error_bound = (3.0 + 16.0 * unit_roundoff) * unit_roundoff;
constexpr double orient3d_error_bound = (7.0 + 56.0 * unit_roundoff) * unit_roundoff;

// Nonoverlapping components: smallest first; only zero is stored as {0}.
using Expansion = std::vector<double>;

double rounded_product(double a,
                       double b) {
    // Preserve product rounding even when FMA contraction is enabled.
    volatile double result = a * b;
    return result;
}

void require_environment() {
    if (std::fegetround() != FE_TONEAREST)
        throw std::runtime_error("Robust arithmetic requires FE_TONEAREST.");

    volatile double minimum_normal = 0x1p-1022;
    volatile double half = 0.5;
    volatile double minimum_subnormal = 0x1p-1074;
    volatile double two = 2.0;

    const double subnormal_result = minimum_normal * half;
    const double subnormal_input_result = minimum_subnormal * two;

    if (std::bit_cast<std::uint64_t>(subnormal_result) != (std::uint64_t{1} << 51) ||
        std::bit_cast<std::uint64_t>(subnormal_input_result) != 2) {
        throw std::runtime_error("Robust arithmetic requires gradual underflow.");
    }
}

void require_finite(double value) {
    if (!std::isfinite(value))
        throw std::invalid_argument("Robust arithmetic requires finite inputs.");
}

ExactPair require_finite_result(ExactPair result) {
    if (!std::isfinite(result.value) || !std::isfinite(result.error))
        throw std::overflow_error("Error-free arithmetic overflowed.");
    return result;
}

ExactPair fast_two_sum_raw(double a,
                           double b) {
    const double value = a + b;
    const double b_virtual = value - a;
    return {value, b - b_virtual};
}

ExactPair two_sum_raw(double a,
                      double b) {
    const double value = a + b;
    const double b_virtual = value - a;
    const double a_virtual = value - b_virtual;
    const double b_error = b - b_virtual;
    const double a_error = a - a_virtual;

    return {value, a_error + b_error};
}

ExactPair two_diff_raw(double a,
                       double b) {
    const double value = a - b;
    const double b_virtual = a - value;
    const double a_virtual = value + b_virtual;
    const double b_error = b_virtual - b;
    const double a_error = a - a_virtual;

    return {value, a_error + b_error};
}

SplitParts split_raw(double a) {
    const double c = rounded_product(splitter, a);
    const double a_big = c - a;
    const double high = c - a_big;

    return {high, a - high};
}

ExactPair two_product_raw(double a,
                          double b) {
    const double value = rounded_product(a, b);

    const auto a_parts = split_raw(a);
    const auto b_parts = split_raw(b);

    const double error_1 = value - rounded_product(a_parts.high, b_parts.high);
    const double error_2 = error_1 - rounded_product(a_parts.low, b_parts.high);
    const double error_3 = error_2 - rounded_product(a_parts.high, b_parts.low);

    return {value, rounded_product(a_parts.low, b_parts.low) - error_3};
}

void require_splittable(double value) {
    if (value != 0.0 && std::fabs(value) < std::numeric_limits<double>::min())
        throw std::underflow_error("Split requires a normal number or zero.");
    if (std::fabs(value) > std::numeric_limits<double>::max() / splitter)
        throw std::overflow_error("Split's intermediate product would overflow.");
}

int least_bit_exponent(double value) {
    const auto bits = std::bit_cast<std::uint64_t>(value);
    const auto exponent = static_cast<int>((bits >> 52) & 0x7ff);
    const auto significand = (bits & ((std::uint64_t{1} << 52) - 1)) | (std::uint64_t{1} << 52);

    return exponent - 1023 - 52 + std::countr_zero(significand);
}

template <std::size_t Size>
void require_coordinates(const std::array<double,
                         Size>& point) {
    for (const double coordinate : point) {
        require_finite(coordinate);

        const double magnitude = std::fabs(coordinate);
        if (magnitude != 0.0 && (magnitude < min_coordinate_magnitude ||
            magnitude > max_coordinate_magnitude))
            throw std::out_of_range("Coordinate magnitude must be zero or in [2^-250, 2^250].");
    }
}

Sign sign_of(double value) {
    if (value > 0.0)
        return Sign::positive;
    if (value < 0.0)
        return Sign::negative;
    return Sign::zero;
}

Expansion from_pair(ExactPair pair) {
    Expansion result;
    if (pair.error != 0.0)
        result.push_back(pair.error);
    if (pair.value != 0.0 || result.empty())
        result.push_back(pair.value);
    return result;
}

Expansion grow_expansion(const Expansion& expansion,
                         double value) {
    Expansion result;
    result.reserve(expansion.size() + 1);

    double carry = value;
    for (const double component : expansion) {
        const auto pair = two_sum_raw(carry, component);
        if (pair.error != 0.0)
            result.push_back(pair.error);

        carry = pair.value;
    }
    if (carry != 0.0 || result.empty())
        result.push_back(carry);

    return result;
}

Expansion expansion_sum(Expansion first,
                        const Expansion& second) {
    for (const double component : second)
        first = grow_expansion(first, component);

    return first;
}

Expansion expansion_difference(Expansion first,
                               const Expansion& second) {
    for (const double component : second)
        first = grow_expansion(first, -component);

    return first;
}

Expansion scale_expansion(const Expansion& expansion,
                          double scalar) {
    Expansion result{0.0};
    for (const double component : expansion) {
        const auto product = two_product_raw(component, scalar);

        result = grow_expansion(result, product.error);
        result = grow_expansion(result, product.value);
    }

    return result;
}

Expansion expansion_product(const Expansion& first,
                            const Expansion& second) {
    Expansion result{0.0};
    for (const double component : second)
        result = expansion_sum(result, scale_expansion(first, component));

    return result;
}

Expansion exact_minor(const Expansion& ax,
                      const Expansion& ay,
                      const Expansion& bx,
                      const Expansion& by) {
    return expansion_difference(expansion_product(ax, by), expansion_product(ay, bx));
}

Sign orient2d_exact(const Coordinates2& a,
                    const Coordinates2& b,
                    const Coordinates2& c) {
    const auto ax = from_pair(two_diff_raw(a[0], c[0]));
    const auto ay = from_pair(two_diff_raw(a[1], c[1]));
    const auto bx = from_pair(two_diff_raw(b[0], c[0]));
    const auto by = from_pair(two_diff_raw(b[1], c[1]));

    return sign_of(exact_minor(ax, ay, bx, by).back());
}

Sign orient3d_exact(const Coordinates3& a,
                    const Coordinates3& b,
                    const Coordinates3& c,
                    const Coordinates3& d) {
    const auto ax = from_pair(two_diff_raw(a[0], d[0]));
    const auto ay = from_pair(two_diff_raw(a[1], d[1]));
    const auto az = from_pair(two_diff_raw(a[2], d[2]));
    const auto bx = from_pair(two_diff_raw(b[0], d[0]));
    const auto by = from_pair(two_diff_raw(b[1], d[1]));
    const auto bz = from_pair(two_diff_raw(b[2], d[2]));
    const auto cx = from_pair(two_diff_raw(c[0], d[0]));
    const auto cy = from_pair(two_diff_raw(c[1], d[1]));
    const auto cz = from_pair(two_diff_raw(c[2], d[2]));

    const auto first = expansion_product(az, exact_minor(bx, by, cx, cy));
    const auto second = expansion_product(bz, exact_minor(cx, cy, ax, ay));
    const auto third = expansion_product(cz, exact_minor(ax, ay, bx, by));

    return sign_of(expansion_sum(expansion_sum(first, second), third).back());
}

} // namespace

ExactPair fast_two_sum(double a,
                       double b) {
    require_environment();
    require_finite(a);
    require_finite(b);

    if (std::fabs(a) < std::fabs(b))
        throw std::invalid_argument("FastTwoSum requires |a| >= |b|.");

    return require_finite_result(fast_two_sum_raw(a, b));
}

ExactPair two_sum(double a,
                  double b) {
    require_environment();
    require_finite(a);
    require_finite(b);

    return require_finite_result(two_sum_raw(a, b));
}

ExactPair two_diff(double a,
                   double b) {
    require_environment();
    require_finite(a);
    require_finite(b);

    return require_finite_result(two_diff_raw(a, b));
}

SplitParts split(double a) {
    require_environment();
    require_finite(a);
    require_splittable(a);

    const auto parts = split_raw(a);
    require_finite_result({parts.high, parts.low});

    return parts;
}

ExactPair two_product(double a,
                      double b) {
    require_environment();
    require_finite(a);
    require_finite(b);
    if (a == 0.0 || b == 0.0)
        return {rounded_product(a, b), 0.0};

    require_splittable(a);
    require_splittable(b);
    if (least_bit_exponent(a) + least_bit_exponent(b) < -1022)
        throw std::underflow_error("TwoProduct's partial products might underflow.");

    return require_finite_result(two_product_raw(a, b));
}

Sign orient2d(const Coordinates2& a,
              const Coordinates2& b,
              const Coordinates2& c) {
    require_environment();
    require_coordinates(a);
    require_coordinates(b);
    require_coordinates(c);

    const double ax = a[0] - c[0];
    const double ay = a[1] - c[1];
    const double bx = b[0] - c[0];
    const double by = b[1] - c[1];

    const double left = rounded_product(ax, by);
    const double right = rounded_product(ay, bx);

    const double determinant = left - right;
    const double permanent = std::fabs(left) + std::fabs(right);
    const double bound = rounded_product(orient2d_error_bound, permanent);
    if (std::fabs(determinant) > bound)
        return sign_of(determinant);

    return orient2d_exact(a, b, c);
}

Sign orient3d(const Coordinates3& a,
              const Coordinates3& b,
              const Coordinates3& c,
              const Coordinates3& d) {
    require_environment();
    require_coordinates(a);
    require_coordinates(b);
    require_coordinates(c);
    require_coordinates(d);

    const double ax = a[0] - d[0];
    const double ay = a[1] - d[1];
    const double az = a[2] - d[2];
    const double bx = b[0] - d[0];
    const double by = b[1] - d[1];
    const double bz = b[2] - d[2];
    const double cx = c[0] - d[0];
    const double cy = c[1] - d[1];
    const double cz = c[2] - d[2];

    const double bx_cy  = rounded_product(bx, cy);
    const double cx_by  = rounded_product(cx, by);
    const double cx_ay  = rounded_product(cx, ay);
    const double ax_cy  = rounded_product(ax, cy);
    const double ax_by  = rounded_product(ax, by);
    const double bx_ay  = rounded_product(bx, ay);
    const double first  = rounded_product(az, bx_cy - cx_by);
    const double second = rounded_product(bz, cx_ay - ax_cy);
    const double third  = rounded_product(cz, ax_by - bx_ay);
    const double determinant = (first + second) + third;

    const double first_bound = rounded_product(std::fabs(az), std::fabs(bx_cy) + std::fabs(cx_by));
    const double second_bound = rounded_product(std::fabs(bz), std::fabs(cx_ay) + std::fabs(ax_cy));
    const double third_bound = rounded_product(std::fabs(cz), std::fabs(ax_by) + std::fabs(bx_ay));

    const double permanent = (first_bound + second_bound) + third_bound;
    const double bound = rounded_product(orient3d_error_bound, permanent);
    
    if (std::fabs(determinant) > bound)
        return sign_of(determinant);

    return orient3d_exact(a, b, c, d);
}

Coordinates3 triangle_cross(const Coordinates3& a,
                            const Coordinates3& b,
                            const Coordinates3& c) {
    require_environment();
    require_coordinates(a);
    require_coordinates(b);
    require_coordinates(c);

    const auto abx = from_pair(two_diff_raw(b[0], a[0]));
    const auto aby = from_pair(two_diff_raw(b[1], a[1]));
    const auto abz = from_pair(two_diff_raw(b[2], a[2]));

    const auto acx = from_pair(two_diff_raw(c[0], a[0]));
    const auto acy = from_pair(two_diff_raw(c[1], a[1]));
    const auto acz = from_pair(two_diff_raw(c[2], a[2]));

    const auto nx = exact_minor(aby, abz, acy, acz);
    const auto ny = exact_minor(abz, abx, acz, acx);
    const auto nz = exact_minor(abx, aby, acx, acy);

    return {nx.back(), ny.back(), nz.back()};
}

} // namespace triangles::robust
