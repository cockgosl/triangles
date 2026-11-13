#ifndef ROBUST_PREDICATE_HPP_
#define ROBUST_PREDICATE_HPP_

#include <array>

namespace triangles::robust {

// Algorithms: https://www.cs.cmu.edu/~quake/robust.html

// value + error is the exact real-number result.
struct ExactPair {
    double value;
    double error;
};

struct SplitParts {
    double high;
    double low;
};

enum class Sign : int {
    negative = -1,
    zero = 0,
    positive = 1
};

using Coordinates2 = std::array<double, 2>;
using Coordinates3 = std::array<double, 3>;

inline constexpr double min_coordinate_magnitude = 0x1p-250;
inline constexpr double max_coordinate_magnitude = 0x1p250;

// Require binary64, round-to-nearest/ties-to-even and gradual underflow.
// Fast-math, reassociation and excess evaluation precision are unsupported.

// Exact a + b; requires |a| >= |b|.
[[nodiscard]] ExactPair fast_two_sum(double a,
                                     double b);

// Exact a + b and a - b.
[[nodiscard]] ExactPair two_sum(double a,
                                double b);
[[nodiscard]] ExactPair two_diff(double a,
                                 double b);

// Requires zero or normal a and finite (2^27 + 1) * a.
[[nodiscard]] SplitParts split(double a);

// Exact a * b; nonzero factors must satisfy split's requirements.
// Their least-bit exponents must sum to >= -1022 to avoid partial underflow.
[[nodiscard]] ExactPair two_product(double a,
                                    double b);

// Predicates accept zero or |coordinate| in [2^-250, 2^250].
// Invalid inputs, unsupported modes and arithmetic range failures throw.

// Sign of det([a-c; b-c]); positive means c is left of a->b, zero collinear.
[[nodiscard]] Sign orient2d(const Coordinates2& a,
                            const Coordinates2& b,
                            const Coordinates2& c);

// Sign of det([a-d; b-d; c-d]); positive means d is below oriented abc.
// Zero also covers degenerate abc.
[[nodiscard]] Sign orient3d(const Coordinates3& a,
                            const Coordinates3& b,
                            const Coordinates3& c,
                            const Coordinates3& d);

// Approximate cross product from exact expansions; preserves component zeroes.
[[nodiscard]] Coordinates3 triangle_cross(const Coordinates3& a,
                                          const Coordinates3& b,
                                          const Coordinates3& c);

} // namespace triangles::robust

#endif //ROBUST_PREDICATE_HPP_
