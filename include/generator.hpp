#pragma once

#include "triangle.hpp"

#include <optional>
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

    Generator(double r_max,
              double expectation,
              double dispersion);

    void gen_x_(Vector3& vector, double length);
    void gen_y_(Vector3& vector, double length);
    void gen_z_(Vector3& vector, double length);

public:
    static std::optional<Generator> Create();
    static std::optional<Generator> Create(double r_max);
    static std::optional<Generator> Create(double r_max,
                                           double expectation);
    static std::optional<Generator> Create(double r_max,
                                           double expectation,
                                           double stddev);

    double get_r_max()  const { return r_max_; }
    double get_expect() const { return expectation_; }
    double get_stddev() const { return stddev_; }

    std::optional<Vector3>   generate_vector();
    std::optional<Vector3>   generate_vector(double length);
    double    generate_length();

    std::optional<Triangle3> generate_triangle();
    std::optional<std::vector<Triangle3>> generate_triangles(unsigned count = GeneratorDefaults::triangles_count);
};

} // namespace triangles
