#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

//#define VULKAN_HPP_NO_SMART_HANDLE
#include "vulkan/vulkan.hpp"
#include <glm/glm.hpp>

#include <functional>


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
  ESCAPE,
  C,
  U
};

enum class ModifierFlagBits
{
  SHIFT = GLFW_MOD_SHIFT,
  CONTROL = GLFW_MOD_CONTROL,
  ALT = GLFW_MOD_ALT,
  SUPER = GLFW_MOD_SUPER,
  CAPS_LOCK = GLFW_MOD_CAPS_LOCK,
  NUM_LOCK = GLFW_MOD_NUM_LOCK
};

inline ModifierFlagBits operator|(ModifierFlagBits a, ModifierFlagBits b)
{
    return static_cast<ModifierFlagBits>(static_cast<int>(a) | static_cast<int>(b));
}

class Window {
 public:
  Window(std::string name, int w, int h);
  ~Window();
  Window(const Window &) = delete;
  Window &operator=(const Window &) = delete;
  std::vector<std::string> getRequiredExtensions();
  const vk::Extent2D& getExtent() const { return _extent; }
  void pollEvents() const;
  void waitEvents() const;
  void setMouseMode(MouseMode mouseMode);
  void setRawMouseMotion(bool enabled);
  bool isRawMouseMotionSupported();

  vk::SurfaceKHR createSurface(vk::Instance instance);

  void addFramebufferResizeListener(std::function<void(vk::Extent2D extent)> onFramebufferResize);
  void addKeyListener(std::function<void(KeyboardKey, bool, vk::Flags<ModifierFlagBits>)> onKey);
  void addCursorPositionListener(std::function<void(glm::dvec2)> onCursorPosition);
  void addCursorEnterListener(std::function<void(bool)> onCursorEnter);
  void addMouseButtonListener(std::function<void(MouseButton, bool, vk::Flags<ModifierFlagBits>)> onMouseButton);
  void addCloseListener(std::function<void()> onClose);

 private:
  std::vector<std::function<void(vk::Extent2D extent)>> _framebufferResizeListeners;
  std::vector<std::function<void(KeyboardKey, bool, vk::Flags<ModifierFlagBits>)>> _keyListeners;
  std::vector<std::function<void(glm::dvec2)>> _cursorPositionListeners;
  std::vector<std::function<void(bool)>> _cursorEnterListeners;
  std::vector<std::function<void(MouseButton, bool, vk::Flags<ModifierFlagBits>)>> _mouseButtonListeners;
  std::vector<std::function<void()>> _closeListeners;

  static void framebufferResizeCallback(GLFWwindow *window, int width, int height);
  static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
  static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
  static void cursorEnterCallback(GLFWwindow* window, int entered);
  static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
  static void closeCallback(GLFWwindow* window);

  vk::Flags<ModifierFlagBits> getModifierFlags(int mods);
  KeyboardKey getKeyboardKey(int key);
  MouseButton getMouseButton(int button);

  std::string windowName;
  GLFWwindow *window; 
  vk::Extent2D _extent;
  
};

}
