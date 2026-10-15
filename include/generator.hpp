#pragma once

#include "triangle.hpp"
#include <random>
#include <vector>

namespace triangles {

struct GeneratorDefaults {
    static constexpr double   r_max               = 1000.0;
    static constexpr double   expectation_divisor = 100.0;
    static constexpr double   stddev_divisor      = 300.0;
    static constexpr unsigned triangles_count     = 4188;
};


class Generator {
private:
    double r_max_;
    double expectation_;
    double stddev_;
    std::mt19937 gen_;

    void gen_x_(Vector3& vector, double length);
    void gen_y_(Vector3& vector, double length);
    void gen_z_(Vector3& vector, double length);

public:
    Generator();
    Generator(double r_max);
    Generator(double r_max,
              double expectation);
    Generator(double r_max,
              double expectation,
              double dispersion);
    ~Generator();

    double get_r_max()  { return r_max_; }
    double get_expect() { return expectation_; }
    double get_stddev() { return stddev_; }

    Vector3   generate_vector();
    Vector3   generate_vector(double length);
    double    generate_length();

    Triangle3 generate_triangle();
    std::vector<Triangle3> generate_triangles(unsigned count = GeneratorDefaults::triangles_count);
};

} // namespace triangles
