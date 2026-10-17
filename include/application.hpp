#ifndef APPLICATION_HPP_
#define APPLICATION_HPP_

#include "glfw_window.hpp"
#include "triangle_renderer.hpp"

#include <optional>

namespace triangles {

class Application {
private:
    bool Initialize();
    void RenderFrame();

    std::optional<::graphics::Window> window_;
    ::graphics::TriangleRenderer renderer_;

public:
    Application() = default;
    ~Application() noexcept = default;

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    int Run();
};

}  // namespace triangles

#endif
