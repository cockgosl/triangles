#include "generator.hpp"

#include <random>
#include <stdexcept>
#include <cmath>
#include <limits>

namespace triangles {

constexpr double EPSILON = std::numeric_limits<double>::epsilon();
// TODO - validate the logic
Generator::Generator(double r_max,
                     double expectation,
                     double stddev)
        :r_max_(r_max), expectation_(expectation), stddev_(stddev) {
    if (std::fabs(r_max_) > GeneratorDefaults::r_max
        || std::fabs(r_max_) <= EPSILON) r_max_ = GeneratorDefaults::r_max;
    // if (r_max_ < 0 && std::fabs(r_max_) > GeneratorDefaults::r_max ) r_max_ = GeneratorDefaults::r_max;

    if (std::fabs(expectation_) >= std::fabs(r_max_)) expectation_ = r_max_ / GeneratorDefaults::expectation_divisor;

    if (stddev_ < 0) stddev_ = -stddev_;
    if (std::fabs(stddev_) >= std::fabs(r_max_) ||
        !std::isfinite(stddev_) ||
        stddev_ <= EPSILON) stddev_ = std::fabs(r_max_ / GeneratorDefaults::stddev_divisor); // TODO - double comparison
}

Generator::~Generator() = default;

double Generator::generate_length() {
    std::normal_distribution<double> norm_gen(expectation_, stddev_);

    double len;
    do {
        len = norm_gen(gen_);
    } while (len < 0.0 || len > r_max_);

    return len;
}

void Generator::gen_x_(Vector3& vector, double length) {
    std::uniform_real_distribution<double> rndm_real(-1.0, 1.0);
    double x_value = length * rndm_real(gen_);

    vector.x = x_value;
}

void Generator::gen_y_(Vector3& vector, double length) {
    std::uniform_real_distribution<double> rndm_real(-1.0, 1.0);
    double rest = std::sqrt(length * length - vector.x * vector.x);
    double y_value = rest * rndm_real(gen_);

    vector.y = y_value;
}

void Generator::gen_z_(Vector3& vector, double length) {
    double rest = std::sqrt(length * length - vector.x * vector.x - vector.y * vector.y);
    std::bernoulli_distribution choose(0.5);
    int sign = choose(gen_) ? 1 : -1;

    double z_value = sign * rest;
    vector.z = z_value;
}

Vector3 Generator::generate_vector() {
    return generate_vector(generate_length());
}

Vector3 Generator::generate_vector(double length) {
    if (!std::isfinite(length) || length < 0.0 || length > r_max_)
        throw std::invalid_argument("length for generating coordinates of vector must be 0 .. r_max");
    Vector3 vector{};

    gen_x_(vector, length);
    gen_y_(vector, length);
    gen_z_(vector, length);

    return vector;
}

Triangle3 Generator::generate_triangle() {
    Triangle3 triangle{};
    std::uniform_real_distribution<double> rndm_len(0.0, r_max_);
    double rndm_length = rndm_len(gen_);

    do {
        Vector3 abs_vector_a = generate_vector(rndm_length);
        Vector3 rel_vector_b = generate_vector();
        Vector3 rel_vector_c = generate_vector();

        triangle.a = abs_vector_a;
        triangle.b = abs_vector_a + rel_vector_b;
        triangle.c = abs_vector_a + rel_vector_c;

    } while(triangle.is_degenerate());

    return triangle;
}

std::vector<Triangle3> Generator::generate_triangles(unsigned count) {
    std::vector<Triangle3> triangle_vec(count);
    for (Triangle3& triangle : triangle_vec)
        triangle = generate_triangle();

    return triangle_vec;
}


} // namespace triangles
