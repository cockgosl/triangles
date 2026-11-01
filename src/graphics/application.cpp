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
#include <iostream>
#include <optional>
#include <ranges>
#include <utility>
#include <vector>

namespace triangles {
namespace {

constexpr double kMaxCoordinateMagnitude = 0.9;

std::optional<std::vector<bool>> FindIntersectingTriangles(
    const std::vector<GeneratedTriangle> &generated_triangles,
    const AABB &domain, const OctotreeConfig &config) {
    auto tree = Octotree::Create(domain, config);

    if (!tree.has_value()) {
        return std::nullopt;
    }

    const std::size_t count = generated_triangles.size();

    for (std::size_t i = 0; i < count; ++i) {
        if (!tree->insert(i, generated_triangles[i].bounds)) {
            return std::nullopt;
        }
    }

    std::vector<bool> intersecting(count, false);

    for (std::size_t i = 0; i < count; ++i) {
        const auto candidates = tree->query(generated_triangles[i].bounds);

        for (const std::size_t j : candidates) {
            if (j <= i) {
                continue;
            }

            const auto result = intersection(generated_triangles[i].triangle,
                                             generated_triangles[j].triangle);

            if (!result.has_value()) {
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

std::vector<float>
ConvertTrianglesToVertices(const std::vector<GeneratedTriangle> &triangles,
                           const std::vector<bool> &intersecting) {
    if (triangles.size() != intersecting.size()) {
        return {};
    }

    constexpr std::array<float, 3> kDefaultColor{1.0f, 0.5f, 0.2f};

    constexpr std::array<float, 3> kIntersectionColor{0.0f, 0.2f, 1.0f};

    std::vector<float> vertices;
    vertices.reserve(triangles.size() * 27);

    for (std::size_t i = 0; i < triangles.size(); ++i) {
        const auto &triangle = triangles[i].triangle;

        const Vector3 normal =
            ((triangle.b - triangle.a).cross(triangle.c - triangle.a))
                .normalized();

        const auto &color =
            intersecting[i] ? kIntersectionColor : kDefaultColor;

        const Vector3 *points[] = {&triangle.a, &triangle.b, &triangle.c};

        for (const Vector3 *point : points) {
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

Application::Application()
    : camera_(glm::vec3{0.0f, 0.0f, 3.0f}, 800.0f / 600.0f,
              ::graphics::CameraConfig{}) {}

bool Application::Initialize() {
    int triangle_count = 0;

    std::cout << "Enter number of triangles: " << std::flush;

    if (!(std::cin >> triangle_count) || triangle_count <= 0) {
        std::cerr << "Invalid number of triangles.\n";
        return false;
    }

    auto window = ::graphics::Window::Create(800, 600, "Triangles");

    if (!window) {
        std::cerr << "Failed to create window.\n";
        return false;
    }

    window_.emplace(std::move(*window));
    window_->SetCursorCaptured(true);

    const auto domain = AABB::Create(Vector3{-2000.0, -2000.0, -2000.0},
                                     Vector3{2000.0, 2000.0, 2000.0});

    if (!domain) {
        std::cerr << "Failed to create generation domain.\n";
        return false;
    }
    const GeneratorConfig generator_config{.domain = *domain,
                                           .target_max_edge_length = 120.0,
                                           .relative_size_variation = 0.15,
                                           .min_shape_quality = 0.1,
                                           .seed = 42u,
                                           .max_shape_attempts = 1000};
    auto generator = Generator::Create(generator_config);

    if (!generator) {
        std::cerr << "Failed to create generator.\n";
        return false;
    }

    auto generated_triangles =
        generator->generate_triangles(static_cast<std::size_t>(triangle_count));

    if (!generated_triangles) {
        std::cerr << "Failed to generate triangles.\n";
        return false;
    }
    auto intersection_results = triangles::FindIntersectingTriangles(
        *generated_triangles, *domain,
        triangles::OctotreeConfig{.max_depth = 8, .max_objects_per_node = 8});

    if (!intersection_results.has_value()) {
        return false;
    }

    auto vertices =
        ConvertTrianglesToVertices(*generated_triangles, *intersection_results);

    if (!renderer_.Initialize(vertices)) {
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

    if (window_->IsKeyPressed(::graphics::Key::kW))
        camera_.Move(::graphics::CameraMovement::kForward, delta_time);

    if (window_->IsKeyPressed(::graphics::Key::kS))
        camera_.Move(::graphics::CameraMovement::kBackward, delta_time);

    if (window_->IsKeyPressed(::graphics::Key::kA))
        camera_.Move(::graphics::CameraMovement::kLeft, delta_time);

    if (window_->IsKeyPressed(::graphics::Key::kD))
        camera_.Move(::graphics::CameraMovement::kRight, delta_time);
}

int Application::Run() {
    if (!Initialize()) {
        return 1;
    }

    auto last_frame_time = std::chrono::steady_clock::now();

    while (!window_->ShouldClose()) {
        window_->PollEvents();

        const auto current_frame_time = std::chrono::steady_clock::now();

        const float delta_time =
            std::chrono::duration<float>(current_frame_time - last_frame_time)
                .count();

        last_frame_time = current_frame_time;

        ProcessInput(delta_time);

        if (!window_->ShouldClose()) {
            RenderFrame();
        }
    }

    return 0;
}

} // namespace triangles
