#include "generator.hpp"

#include <cmath>
#include <limits>
#include <optional>
#include <random>

namespace triangles {

constexpr double EPSILON = std::numeric_limits<double>::epsilon(); //TODO - get rid of the internal interlinkage variable
// TODO - validate the logic

Generator::Generator(double r_max,
                     double expectation,
                     double stddev)
    : r_max_(r_max),
      expectation_(expectation),
      stddev_(stddev) {}

std::optional<Generator> Generator::Create() {
    return Create(GeneratorDefaults::r_max);
}

std::optional<Generator> Generator::Create(double r_max) {
    return Create(r_max,
                  r_max / GeneratorDefaults::expectation_divisor);
}

std::optional<Generator> Generator::Create(double r_max,
                                           double expectation) {
    return Create(r_max,
                  expectation,
                  r_max / GeneratorDefaults::stddev_divisor);
}

std::optional<Generator> Generator::Create(double r_max,
                                           double expectation,
                                           double stddev) {
    if (!std::isfinite(r_max) ||
        r_max <= 0.0 ||
        r_max > GeneratorDefaults::r_max) {
        return std::nullopt;
    }

    if (!std::isfinite(expectation) ||
        expectation < 0.0 ||
        expectation >= r_max) {
        return std::nullopt;
    }

    if (!std::isfinite(stddev) ||
        stddev <= 0.0 ||
        stddev >= r_max) {
        return std::nullopt;
    }

    return Generator(r_max, expectation, stddev);
}

double Generator::generate_length() {
    std::normal_distribution<double> norm_gen(expectation_, stddev_);

    double len;
    do {
        len = norm_gen(gen_);
    } while (len < EPSILON || len > std::fabs(r_max_));

    return len;
}

std::optional<Vector3> Generator::generate_vector() {
  return generate_vector(generate_length());
}

std::optional<Vector3> Generator::generate_vector(double length) {
    if (!std::isfinite(length) || length < 0.0 || length > r_max_) {
      return std::nullopt;
    }
  
    if (length == 0.0) {
      return Vector3{};
    }
  
    std::uniform_real_distribution<double> z_distribution(-1.0, 1.0);
    std::uniform_real_distribution<double> angle_distribution(
        0.0, 2.0 * std::numbers::pi_v<double>);
  
    const double z_unit = z_distribution(gen_);
    const double angle = angle_distribution(gen_);
    const double xy_radius = std::sqrt(1.0 - z_unit * z_unit);
  
    return Vector3(
        length * xy_radius * std::cos(angle),
        length * xy_radius * std::sin(angle),
        length * z_unit);
}

std::optional<Triangle3> Generator::generate_triangle() {
    Triangle3 triangle{};
    std::uniform_real_distribution<double> rndm_len(0.0, r_max_);
    double rndm_length = rndm_len(gen_);
  
    do {
      std::optional<Vector3> abs_vector_a = generate_vector(rndm_length);
      std::optional<Vector3> rel_vector_b = generate_vector();
      std::optional<Vector3> rel_vector_c = generate_vector();
  
      if (!abs_vector_a || !rel_vector_b || !rel_vector_c) {
        return std::nullopt;
      }
  
      triangle.a = *abs_vector_a;
      triangle.b = *abs_vector_a + *rel_vector_b;
      triangle.c = *abs_vector_a + *rel_vector_c;
  
    } while (triangle.is_degenerate());
  
    return triangle;
}
std::optional<std::vector<Triangle3>> Generator::generate_triangles(unsigned count) {
    std::vector<Triangle3> result;
    result.reserve(count);
  
    for (unsigned index = 0; index < count; ++index) {
      auto triangle = generate_triangle();
  
      if (!triangle) {
        return std::nullopt;
      }
  
      result.push_back(*triangle);
    }
  
    return result;
}

} // namespace triangles
