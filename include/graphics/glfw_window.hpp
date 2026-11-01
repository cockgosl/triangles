#ifndef GLFW_WINDOW_HPP_
#define GLFW_WINDOW_HPP_

#include <optional>
#include <utility>

struct GLFWwindow;

namespace graphics {

enum class Key { kW, kA, kS, kD, kEscape };

class Window {
  private:
    explicit Window(GLFWwindow *window) noexcept;

    void Reset() noexcept;

    GLFWwindow *window_ = nullptr;
    bool glfw_initialized_ = false;

    static void CursorPositionCallback(GLFWwindow *window, double x_position,
                                       double y_position);

    double last_cursor_x_ = 0.0;
    double last_cursor_y_ = 0.0;

    double mouse_delta_x_ = 0.0;
    double mouse_delta_y_ = 0.0;

  public:
    static std::optional<Window> Create(int width, int height,
                                        const char *title);

    ~Window() noexcept;

    Window(const Window &) = delete;
    Window &operator=(const Window &) = delete;

    Window(Window &&rhs) noexcept;
    Window &operator=(Window &&rhs) = delete;

    bool ShouldClose() const noexcept;
    void SwapBuffers() const noexcept;
    void PollEvents() const noexcept;

    bool IsKeyPressed(Key key) const noexcept;
    void RequestClose() const noexcept;

    std::pair<double, double> ConsumeMouseDelta() noexcept;
    void SetCursorCaptured(bool captured) noexcept;
};

} // namespace graphics

#endif
