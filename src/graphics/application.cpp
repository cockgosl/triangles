#include "graphics/application.hpp"

#include "geometry/generator.hpp"
#include "geometry/intersection.hpp"
#include "geometry/octotree.hpp"
#include "geometry/triangle.hpp"
#include "geometry/vector3.hpp"

#include <glad/gl.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace triangles {
namespace {

std::optional<AABB> GetSceneBounds(const std::vector<SceneTriangle>& triangles) {
    if (triangles.empty()) return std::nullopt;

    Vector3 min = triangles.front().bounds.get_min();
    Vector3 max = triangles.front().bounds.get_max();

    for (const auto& triangle : triangles) {
        const auto& lower = triangle.bounds.get_min();
        const auto& upper = triangle.bounds.get_max();

        min = {std::min(min.x, lower.x), std::min(min.y, lower.y), std::min(min.z, lower.z)};
        max = {std::max(max.x, upper.x), std::max(max.y, upper.y), std::max(max.z, upper.z)};
    }
    return AABB::Create(min, max);
}

std::optional<AABB> GetOctotreeDomain(const AABB& bounds) {
    const Vector3 center = bounds.center();
    const Vector3 extent = bounds.extent();

    const double half_extent = std::max({extent.x, extent.y, extent.z}) * 0.5;

    const auto lower = [half_extent](double value, double min) {
        return std::nextafter(std::min(min, value - half_extent), -std::numeric_limits<double>::infinity());
    };
    const auto upper = [half_extent](double value, double max) {
        return std::nextafter(std::max(max, value + half_extent), std::numeric_limits<double>::infinity());
    };

    const auto& min = bounds.get_min();
    const auto& max = bounds.get_max();

    return AABB::Create({lower(center.x, min.x), lower(center.y, min.y), lower(center.z, min.z)},
                        {upper(center.x, max.x), upper(center.y, max.y), upper(center.z, max.z)});
}

std::optional<::graphics::ShadowConfig> GetShadowConfig(const AABB& bounds) {
    ::graphics::ShadowConfig shadow_config;
    const Vector3 center = bounds.center();

    const double light_distance = std::max(bounds.extent().length(), 1.0);

    const Vector3 light_direction{shadow_config.light_position.x,
                                  shadow_config.light_position.y,
                                  shadow_config.light_position.z};

    const Vector3 light_position = center + light_direction.normalized() * light_distance;

    const double light_magnitude = std::max({std::abs(light_position.x), 
                                             std::abs(light_position.y), 
                                             std::abs(light_position.z)});

    if (light_magnitude > std::numeric_limits<float>::max()) return std::nullopt;

    shadow_config.light_position = {static_cast<float>(light_position.x),
                                    static_cast<float>(light_position.y),
                                    static_cast<float>(light_position.z)};

    shadow_config.light_target   = {static_cast<float>(center.x),
                                    static_cast<float>(center.y), 
                                    static_cast<float>(center.z)};

    return shadow_config;
}

std::optional<std::vector<bool>> FindIntersectingTriangles(const std::vector<SceneTriangle>& triangles, const AABB& domain,
                                                           const OctotreeConfig& config) {
    std::string error;
    auto tree = Octotree::Create(domain, config, &error);

    if (!tree) {
        std::cerr << "Octotree configuration error: " << error << '\n';
        return std::nullopt;
    }

    const std::size_t count = triangles.size();
    for (std::size_t i = 0; i < count; ++i) {
        if (!tree->insert(i, triangles[i].bounds)) {
            std::cerr << "Failed to insert triangle " << i + 1 << " into octotree.\n";
            return std::nullopt;
        }
    }

    std::vector<bool> intersecting(count, false);
    for (std::size_t i = 0; i < count; ++i) {
        const auto candidates = tree->query(triangles[i].bounds);

        for (const std::size_t j : candidates) {
            if (j <= i) continue;
            const auto result = intersection(triangles[i].triangle, triangles[j].triangle);

            if (!result.has_value()) {
                std::cerr << "Failed to classify triangles " << i + 1 << " and " << j + 1 << ".\n";
                return std::nullopt;
            }
            if (*result) {
                intersecting[i] = true;
                intersecting[j] = true;
            }
        }
    }
    return intersecting;
}

std::vector<float> ConvertTrianglesToVertices(const std::vector<SceneTriangle>& triangles, const std::vector<bool>& intersecting) {
    if (triangles.size() != intersecting.size()) return {};

    constexpr std::array<float, 3> kDefaultColor{1.0f, 0.5f, 0.2f};
    constexpr std::array<float, 3> kIntersectionColor{0.0f, 0.2f, 1.0f};
    std::vector<float> vertices;
    vertices.reserve(triangles.size() * 27);

    for (std::size_t i = 0; i < triangles.size(); ++i) {
        const auto& triangle = triangles[i].triangle;
        const Vector3 normal = triangle.normal();

        const auto& color = intersecting[i] ? kIntersectionColor : kDefaultColor;
        const Vector3* points[] = {&triangle.a, &triangle.b, &triangle.c};

        for (const Vector3* point : points) {
            const double magnitude = std::max({std::abs(point->x), std::abs(point->y), std::abs(point->z)});
            if (magnitude > std::numeric_limits<float>::max()) return {};
            vertices.push_back(static_cast<float>(point->x));
            vertices.push_back(static_cast<float>(point->y));
            vertices.push_back(static_cast<float>(point->z));

            vertices.push_back(static_cast<float>(normal.x));
            vertices.push_back(static_cast<float>(normal.y));
            vertices.push_back(static_cast<float>(normal.z));

            vertices.push_back(color[0]);
            vertices.push_back(color[1]);
            vertices.push_back(color[2]);
        }
    }

    return vertices;
}

} // namespace

Application::Application(TriangleSource source)
    : source_(source), camera_(glm::vec3{0.0f, 0.0f, 3.0f}, 800.0f / 600.0f, ::graphics::CameraConfig{}) {}


bool Application::Initialize() {
    if (source_ == TriangleSource::kGenerator) std::cout << "Enter number of triangles: " << std::flush;
    else std::cout << "Enter number N of triangles and their coordinates: " << std::flush;

    std::string error;
    const auto triangle_count = read_triangle_count(std::cin, &error);
    if (!triangle_count) {
        std::cerr << error << '\n';
        return false;
    }

    std::optional<std::vector<SceneTriangle>> triangles;
    if (source_ == TriangleSource::kInput) {
        triangles = read_triangles(std::cin, *triangle_count, &error);
    } else {
        const auto domain = AABB::Create(Vector3{-2000.0, -2000.0, -2000.0}, Vector3{2000.0, 2000.0, 2000.0});
        if (!domain) {
            std::cerr << "Failed to create generation domain.\n";
            return false;
        }

        const GeneratorConfig config{
            .domain = *domain,
            .target_max_edge_length = 100.0,
            .relative_size_variation = 0.2,
            .min_shape_quality = 0.15,
            .seed = 5489u,
            .max_shape_attempts = 1000,
        };

        auto generator = Generator::Create(config, &error);
        if (generator) {
            triangles = generator->generate_triangles(*triangle_count);
            if (!triangles) error = "Failed to generate triangles.";
        }
    }

    if (!triangles) {
        std::cerr << error << '\n';
        return false;
    }

    const auto bounds = GetSceneBounds(*triangles);
    const auto domain = bounds ? GetOctotreeDomain(*bounds) : std::nullopt;
    if (!domain) {
        std::cerr << "Failed to create scene domain.\n";
        return false;
    }

    const OctotreeConfig config{.max_depth = 8, .max_objects_per_node = 16};
    auto intersection_results = FindIntersectingTriangles(*triangles, *domain, config);
    if (!intersection_results) return false;

    auto vertices = ConvertTrianglesToVertices(*triangles, *intersection_results);
    if (vertices.empty()) {
        std::cerr << "Failed to create scene vertices: coordinates must fit in float.\n";
        return false;
    }

    const auto shadow_config = GetShadowConfig(*bounds);
    if (!shadow_config) {
        std::cerr << "Scene bounds exceed the lighting coordinate range.\n";
        return false;
    }

    auto window = ::graphics::Window::Create(800, 600, "Triangles");
    if (!window) {
        std::cerr << "Failed to create window.\n";
        return false;
    }

    window_.emplace(std::move(*window));
    window_->SetCursorCaptured(true);

    if (!renderer_.Initialize(vertices, *shadow_config)) {
        std::cerr << "Failed to initialize renderer.\n";
        return false;
    }

    return true;
}

void Application::RenderFrame() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const glm::mat4 view = camera_.GetViewMatrix();
    const glm::mat4 projection = camera_.GetProjectionMatrix();

    renderer_.Render(view, projection);

    window_->SwapBuffers();
}

void Application::ProcessInput(float delta_time) {
    if (window_->IsKeyPressed(::graphics::Key::kEscape)) {
        window_->RequestClose();
        return;
    }

    const auto [x_offset, y_offset] = window_->ConsumeMouseDelta();
    camera_.Rotate(static_cast<float>(x_offset), static_cast<float>(y_offset));

    if (window_->IsKeyPressed(::graphics::Key::kW)) camera_.Move(::graphics::CameraMovement::kForward, delta_time);
    if (window_->IsKeyPressed(::graphics::Key::kS)) camera_.Move(::graphics::CameraMovement::kBackward, delta_time);
    if (window_->IsKeyPressed(::graphics::Key::kA)) camera_.Move(::graphics::CameraMovement::kLeft, delta_time);
    if (window_->IsKeyPressed(::graphics::Key::kD)) camera_.Move(::graphics::CameraMovement::kRight, delta_time);
}

int Application::Run() {
    if (!Initialize()) return EXIT_FAILURE;

    auto last_frame_time = std::chrono::steady_clock::now();

    while (!window_->ShouldClose()) {
        window_->PollEvents();

        const auto current_frame_time = std::chrono::steady_clock::now();

        const float delta_time = std::chrono::duration<float>(current_frame_time - last_frame_time).count();

        last_frame_time = current_frame_time;

        ProcessInput(delta_time);

        if (!window_->ShouldClose()) RenderFrame();
    }

    return EXIT_SUCCESS;
}

} // namespace triangles
