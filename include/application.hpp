#ifndef APPLICATION_HPP_
#define APPLICATION_HPP_

#include "camera.hpp"
#include "glfw_window.hpp"
#include "triangle_renderer.hpp"

#include <optional>

namespace triangles {

class Application {
  private:
    bool Initialize();
    void RenderFrame();
    void ProcessInput(float delta_time);

    std::optional<::graphics::Window> window_;
    ::graphics::Camera camera_;
    ::graphics::TriangleRenderer renderer_;

  public:
    Application();
    ~Application() noexcept = default;

    Application(const Application &) = delete;
    Application &operator=(const Application &) = delete;

    int Run();
};

} // namespace triangles

#endif
