#include "window.hpp"

// std
#include <stdexcept>

Window::Window(int w, int h, std::string name) : width{w}, height{h}, windowName{name} {
  initWindow();
}

Window::~Window() {
  glfwDestroyWindow(window);
  glfwTerminate();
}

void Window::initWindow() {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
  glfwSetWindowUserPointer(window, this);
  glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

vk::raii::SurfaceKHR Window::createWindowSurface(vk::raii::Instance &instance) {
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(VkInstance(*instance), window, nullptr, &surface) !=
      VK_SUCCESS) {
        throw std::runtime_error("failed to craete window surface");
    }
    return vk::raii::SurfaceKHR(instance, surface);
}

void Window::framebufferResizeCallback(GLFWwindow *window, int width, int height) {
  auto w = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
  w->framebufferResized = true;
  w->width = width;
  w->height = height;
}
