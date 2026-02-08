#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

//#define VULKAN_HPP_NO_SMART_HANDLE
#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_raii.hpp"
#include <glm/glm.hpp>



#include <string>

namespace engine {

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
  Window(const vk::Instance& instance, std::string name, int w, int h);
  ~Window();
  Window(const Window &) = delete;
  Window &operator=(const Window &) = delete;

  std::vector<const char *> getRequiredExtensions();
  bool shouldClose() { return glfwWindowShouldClose(window); }
  const vk::Extent2D& getExtent() { return _extent; }
  const vk::SurfaceKHR& getSurface();
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

 private:
  static void framebufferResizeCallback(GLFWwindow *window, int width, int height);
  void initWindow(const vk::Instance& instance);

  int width;
  int height;
  bool framebufferResized = false;

  std::string windowName;
  GLFWwindow *window;
  vk::SurfaceKHR _surface; 
  vk::Extent2D _extent;
  
};

}
