#include "glfw_window.hpp"

#include <glad/gl.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iostream>
#include <utility>

namespace graphics {
namespace {

void FramebufferSizeCallback(
    GLFWwindow*,
    int width,
    int height) {
    glViewport(0, 0, width, height);
}

}  // namespace

Window::Window(GLFWwindow* window) noexcept
    : window_(window),
      glfw_initialized_(true) {}

std::optional<Window> Window::Create(
    int width,
    int height,
    const char* title) {
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

    GLFWwindow* window = glfwCreateWindow(width, height, title, nullptr, nullptr);

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

    glfwGetFramebufferSize(
        window, &framebuffer_width, &framebuffer_height);

    glViewport(0, 0, framebuffer_width, framebuffer_height);

    return Window(window);
}

Window::~Window() noexcept { Reset(); }

Window::Window(Window&& rhs) noexcept : 
    window_(std::exchange(rhs.window_, nullptr)),
    glfw_initialized_(std::exchange(rhs.glfw_initialized_, false)) {}

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
    return window_ == nullptr ||
           glfwWindowShouldClose(window_) == GLFW_TRUE;
}

void Window::SwapBuffers() const noexcept { if (window_ != nullptr) glfwSwapBuffers(window_); }

void Window::PollEvents() const noexcept { if (glfw_initialized_) glfwPollEvents(); }

}  // namespace graphics
