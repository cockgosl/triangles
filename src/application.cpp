#include "application.hpp"

#include "generator.hpp"
#include "triangle.hpp"
#include "vector3.hpp"

#include <glad/gl.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <utility>
#include <vector>

namespace triangles {
namespace {

constexpr double kMaxCoordinateMagnitude = 0.9;

std::vector<float>
ConvertTrianglesToVertices(const std::vector<Triangle3> &triangles) {
    double max_abs_coordinate = 0.0;

    for (const auto &triangle : triangles) {
        const Vector3 *points[] = {&triangle.a, &triangle.b, &triangle.c};

        for (const auto *point : points) {
            max_abs_coordinate =
                std::max({max_abs_coordinate, std::abs(point->x),
                          std::abs(point->y), std::abs(point->z)});
        }
    }

    double scale = 1.0;

    if (max_abs_coordinate > kMaxCoordinateMagnitude) {
        scale = kMaxCoordinateMagnitude / max_abs_coordinate;
    }

    std::vector<float> vertices;
    vertices.reserve(triangles.size() * 9);

    for (const auto &triangle : triangles) {
        const Vector3 *points[] = {&triangle.a, &triangle.b, &triangle.c};

        for (const auto *point : points) {
            vertices.push_back(static_cast<float>(point->x * scale));
            vertices.push_back(static_cast<float>(point->y * scale));
            vertices.push_back(static_cast<float>(point->z * scale));
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

    auto generator = Generator::Create();

    if (!generator) {
        std::cerr << "Failed to create generator.\n";
        return false;
    }

    auto generated_triangles =
        generator->generate_triangles(static_cast<unsigned>(triangle_count));

    if (!generated_triangles) {
        std::cerr << "Failed to generate triangles.\n";
        return false;
    }

    const std::vector<float> vertices =
        ConvertTrianglesToVertices(*generated_triangles);

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

    if (window_->IsKeyPressed(::graphics::Key::kW)) {
        camera_.Move(::graphics::CameraMovement::kForward, delta_time);
    }

    if (window_->IsKeyPressed(::graphics::Key::kS)) {
        camera_.Move(::graphics::CameraMovement::kBackward, delta_time);
    }

    if (window_->IsKeyPressed(::graphics::Key::kA)) {
        camera_.Move(::graphics::CameraMovement::kLeft, delta_time);
    }

    if (window_->IsKeyPressed(::graphics::Key::kD)) {
        camera_.Move(::graphics::CameraMovement::kRight, delta_time);
    }
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
