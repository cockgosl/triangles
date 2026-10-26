#include "glfw_window.hpp"

#include <glad/gl.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iostream>
#include <utility>

namespace graphics {
namespace {

void FramebufferSizeCallback(GLFWwindow *, int width, int height) {
    glViewport(0, 0, width, height);
}

} // namespace

Window::Window(GLFWwindow *window) noexcept
    : window_(window), glfw_initialized_(true) {
    glfwSetWindowUserPointer(window_, this);

    glfwSetCursorPosCallback(window_, CursorPositionCallback);
}

std::optional<Window> Window::Create(int width, int height, const char *title) {
    if (width <= 0 || height <= 0 || title == nullptr) {
        std::cerr << "Invalid window parameters.\n";
        return std::nullopt;
    }

    if (glfwInit() != GLFW_TRUE) {
        std::cerr << "Failed to initialize GLFW.\n";
        return std::nullopt;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window =
        glfwCreateWindow(width, height, title, nullptr, nullptr);

    if (window == nullptr) {
        std::cerr << "Failed to create GLFW window.\n";
        glfwTerminate();
        return std::nullopt;
    }

    glfwMakeContextCurrent(window);

    if (gladLoadGL(reinterpret_cast<GLADloadfunc>(glfwGetProcAddress)) == 0) {
        std::cerr << "Failed to initialize GLAD.\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return std::nullopt;
    }

    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);

    int framebuffer_width = 0;
    int framebuffer_height = 0;

    glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);

    glViewport(0, 0, framebuffer_width, framebuffer_height);

    return Window(window);
}

Window::~Window() noexcept { Reset(); }

Window::Window(Window &&other) noexcept
    : window_(std::exchange(other.window_, nullptr)),
      glfw_initialized_(std::exchange(other.glfw_initialized_, false)) {
    if (window_ != nullptr) {
        glfwSetWindowUserPointer(window_, this);
    }
}

void Window::Reset() noexcept {
    if (window_ != nullptr) {
        glfwDestroyWindow(window_);
        window_ = nullptr;
    }

    if (glfw_initialized_) {
        glfwTerminate();
        glfw_initialized_ = false;
    }
}

bool Window::ShouldClose() const noexcept {
    return window_ == nullptr || glfwWindowShouldClose(window_) == GLFW_TRUE;
}

void Window::SwapBuffers() const noexcept {
    if (window_ != nullptr)
        glfwSwapBuffers(window_);
}

void Window::PollEvents() const noexcept {
    if (glfw_initialized_)
        glfwPollEvents();
}

void Window::SetCursorCaptured(bool captured) noexcept {
    if (window_ == nullptr) {
        return;
    }

    if (captured) {
        glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        glfwGetCursorPos(window_, &last_cursor_x_, &last_cursor_y_);

        mouse_delta_x_ = 0.0;
        mouse_delta_y_ = 0.0;
    } else {
        glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

void Window::CursorPositionCallback(GLFWwindow *window, double x_position,
                                    double y_position) {
    auto *self = static_cast<Window *>(glfwGetWindowUserPointer(window));

    if (self == nullptr) {
        return;
    }

    self->mouse_delta_x_ += x_position - self->last_cursor_x_;

    self->mouse_delta_y_ += self->last_cursor_y_ - y_position;

    self->last_cursor_x_ = x_position;
    self->last_cursor_y_ = y_position;
}

std::pair<double, double> Window::ConsumeMouseDelta() noexcept {
    const std::pair<double, double> delta{mouse_delta_x_, mouse_delta_y_};

    mouse_delta_x_ = 0.0;
    mouse_delta_y_ = 0.0;

    return delta;
}

int ToGlfwKey(Key key) {
    switch (key) {
    case Key::kW:
        return GLFW_KEY_W;
    case Key::kA:
        return GLFW_KEY_A;
    case Key::kS:
        return GLFW_KEY_S;
    case Key::kD:
        return GLFW_KEY_D;
    case Key::kEscape:
        return GLFW_KEY_ESCAPE;
    }

    return GLFW_KEY_UNKNOWN;
}

bool Window::IsKeyPressed(Key key) const noexcept {
    if (window_ == nullptr) {
        return false;
    }

    const int glfw_key = ToGlfwKey(key);

    if (glfw_key == GLFW_KEY_UNKNOWN) {
        return false;
    }

    return glfwGetKey(window_, glfw_key) == GLFW_PRESS;
}

void Window::RequestClose() const noexcept {
    if (window_ != nullptr) {
        glfwSetWindowShouldClose(window_, GLFW_TRUE);
    }
}

} // namespace graphics
