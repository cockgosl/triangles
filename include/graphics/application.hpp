#ifndef APPLICATION_HPP_
#define APPLICATION_HPP_

#include "camera.hpp"
#include "glfw_window.hpp"
#include "triangle_renderer.hpp"

#include <optional>

namespace triangles {

enum class TriangleSource {
    kGenerator,
    kInput
};

class Application {
private:
    bool Initialize();
    void RenderFrame();
    void ProcessInput(float delta_time);

    TriangleSource source_;
    bool console_;
    std::optional<::graphics::Window> window_;
    ::graphics::Camera camera_;
    ::graphics::TriangleRenderer renderer_;

public:
    explicit Application(TriangleSource source = TriangleSource::kInput, bool console = false);
    ~Application() noexcept = default;

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    int Run();
};

} // namespace triangles

#endif
