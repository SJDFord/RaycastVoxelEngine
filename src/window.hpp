#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

//#define VULKAN_HPP_NO_SMART_HANDLE
#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_raii.hpp"
#include <glm/glm.hpp>



#include <string>

enum class MouseMode { NORMAL, HIDDEN, DISABLED };
enum class MouseButton { LEFT, RIGHT };

	enum class KeyboardKey {
  A,
  D,
  W,
  S,
  SPACE,
  LEFT_SHIFT,
  LEFT,
  RIGHT,
  UP,
  DOWN,
  ESCAPE
};

class Window {
 public:
  Window(int w, int h, std::string name);
  ~Window();
  Window(const Window &) = delete;
  Window &operator=(const Window &) = delete;

  std::vector<const char *> getRequiredExtensions();
  bool shouldClose() { return glfwWindowShouldClose(window); }
  VkExtent2D getExtent() { return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}; }
  bool wasWindowResized() { return framebufferResized; }
  void resetWindowResizedFlag() { framebufferResized = false; }
  void pollEvents();
  void waitEvents();
  void setMouseMode(MouseMode mouseMode);
  void setRawMouseMotion(bool enabled);
  void getMousePosition(glm::vec2 &result);
  // TODO: method for registering a callback for mouse button presses
  bool isMouseButtonPressed(MouseButton mouseButton);
  // TODO: method for registering a callback for keypresses
  bool isKeyPressed(KeyboardKey keyboardKey);
  void close();

  vk::raii::SurfaceKHR createWindowSurface(vk::raii::Instance &instance);

 private:
  static void framebufferResizeCallback(GLFWwindow *window, int width, int height);
  void initWindow();

  int width;
  int height;
  bool framebufferResized = false;

  std::string windowName;
  GLFWwindow *window;
};
