#ifndef GLFW_WINDOW_HPP_
#define GLFW_WINDOW_HPP_

#include <optional>

struct GLFWwindow;

namespace graphics {

class Window {
private:
    explicit Window(GLFWwindow* window) noexcept;

    void Reset() noexcept;

    GLFWwindow* window_ = nullptr;
    bool glfw_initialized_ = false;

public:
    static std::optional<Window> Create(
        int width,
        int height,
        const char* title);

    ~Window() noexcept;

    Window(const Window&) = delete;
    Window& operator=(const Window&)  = delete;

    Window(Window&& other) noexcept;
    Window& operator=(Window&& other) = delete;

    bool ShouldClose() const noexcept;
    void SwapBuffers() const noexcept;
    void PollEvents()  const noexcept;
};

}  // namespace graphics

#endif
