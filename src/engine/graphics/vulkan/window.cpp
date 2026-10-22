#include "window.hpp"

// std
#include <stdexcept>
#include <print>

namespace engine {

Window::Window(std::string name, int w, int h) : windowName{name}, _extent{vk::Extent2D( static_cast<uint32_t>(w), static_cast<uint32_t>(h) )} {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  window = glfwCreateWindow(_extent.width, _extent.height, windowName.c_str(), nullptr, nullptr);
  glfwSetWindowUserPointer(window, this);
  glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
  glfwSetKeyCallback(window, keyCallback);
  glfwSetCursorPosCallback(window, cursorPositionCallback);
  glfwSetCursorEnterCallback(window, cursorEnterCallback);
  glfwSetMouseButtonCallback(window, mouseButtonCallback);
  glfwSetWindowCloseCallback(window, closeCallback);
  // TODO: Position window in the center of the screen using glfwSetWindowPos
}

Window::~Window() {
  glfwDestroyWindow(window);
  glfwTerminate();
}

vk::SurfaceKHR Window::createSurface(vk::Instance instance) {
  VkSurfaceKHR surface;
  VkResult     err = glfwCreateWindowSurface( instance, window, nullptr, &surface );
  if ( err != VK_SUCCESS ) {
    throw std::runtime_error( "Failed to create window!" );
  }
  return surface;
    //_surface = vk::SurfaceKHR( surface );
}

void Window::addFramebufferResizeListener(std::function<void(vk::Extent2D extent)> onFramebufferResize) {
  _framebufferResizeListeners.push_back(onFramebufferResize);
}

void Window::addKeyListener(std::function<void(KeyboardKey, bool, vk::Flags<ModifierFlagBits>)> onKey) {
  _keyListeners.push_back(onKey);
}

void Window::addCursorPositionListener(std::function<void(glm::dvec2)> onCursorPosition) {
  _cursorPositionListeners.push_back(onCursorPosition);
}

void Window::addCursorEnterListener(std::function<void(bool)> onCursorEnter) {
  _cursorEnterListeners.push_back(onCursorEnter);
}

std::vector<std::string>  Window::getRequiredExtensions() {
  uint32_t glfwExtensionCount = 0;
  const char **glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector<std::string> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
  return extensions;
}


void Window::addMouseButtonListener(std::function<void(MouseButton, bool, vk::Flags<ModifierFlagBits>)> onMouseButton) {
  _mouseButtonListeners.push_back(onMouseButton);
}

void Window::addCloseListener(std::function<void()> onClose) {
  _closeListeners.push_back(onClose);
}

void Window::framebufferResizeCallback(GLFWwindow *window, int width, int height) {
  Window* handler = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
  auto extent = vk::Extent2D( static_cast<uint32_t>(width), static_cast<uint32_t>(height) );
  if (handler) {
    handler->_extent = extent;
    for (auto listener : handler->_framebufferResizeListeners) {
      listener(extent);
    }
  }
}

void Window::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
  Window* handler = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
  if (handler) {
    try {
      //std::println("Key {} {} {} {}", key, scancode, action, mods);
      KeyboardKey keyboardKey = handler->getKeyboardKey(key);
      bool isPressed = (action & GLFW_PRESS) | (action & GLFW_REPEAT);
      vk::Flags<ModifierFlagBits> flags = handler->getModifierFlags(mods);
      for (auto listener : handler->_keyListeners) {
        listener(keyboardKey, isPressed, flags);
      } 
    } catch (const std::logic_error& e) {
      std::println("Unimplemented key pressed");
    }
  }
}

void Window::cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
  Window* handler = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
  if (handler) {
    glm::dvec2 pos = {xpos, ypos};
    for (auto listener : handler->_cursorPositionListeners) {
      listener(pos);
    }
  }
}

void Window::cursorEnterCallback(GLFWwindow* window, int entered) {
  Window* handler = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
  if (handler) {
    for (auto listener : handler->_cursorEnterListeners) {
      listener(entered);
    }
  }
}


void Window::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
  Window* handler = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
  if (handler) {
    try {
      //std::println("Mouse Button {} {} {}", button, action, mods);
      MouseButton mouseButton = handler->getMouseButton(button);
      bool isPressed = (action & GLFW_PRESS);
      vk::Flags<ModifierFlagBits> flags = handler->getModifierFlags(mods);    
      for (auto listener : handler->_mouseButtonListeners) {
        listener(mouseButton, isPressed, flags);
      }
    } catch (const std::logic_error& e) {
      std::println("Unimplemented mouse button pressed");
    }
  }
}

vk::Flags<ModifierFlagBits> Window::getModifierFlags(int mods) {

  vk::Flags<ModifierFlagBits> flags;

  if (mods & GLFW_MOD_SHIFT) {
    flags = flags | ModifierFlagBits::SHIFT;
  }

  if (mods & GLFW_MOD_CONTROL) {
    flags = flags | ModifierFlagBits::CONTROL;
  }

  if (mods & GLFW_MOD_ALT) {
    flags = flags | ModifierFlagBits::ALT;
  }

  if (mods & GLFW_MOD_SUPER) {
    flags = flags | ModifierFlagBits::SUPER;
  }

  if (mods & GLFW_MOD_CAPS_LOCK) {
    flags = flags | ModifierFlagBits::CAPS_LOCK;
  }

  if (mods & GLFW_MOD_NUM_LOCK) {
    flags = flags | ModifierFlagBits::NUM_LOCK;
  }

  return flags;
}

KeyboardKey Window::getKeyboardKey(int key) {
  // TODO: Maybe generate this instead? 
  switch (key) { 
    case GLFW_KEY_A: return KeyboardKey::A;
    case GLFW_KEY_D: return KeyboardKey::D;
    case GLFW_KEY_S: return KeyboardKey::S;
    case GLFW_KEY_W: return KeyboardKey::W;
    case GLFW_KEY_LEFT: return KeyboardKey::LEFT;
    case GLFW_KEY_RIGHT: return KeyboardKey::RIGHT;
    case GLFW_KEY_UP: return KeyboardKey::UP;
    case GLFW_KEY_DOWN: return KeyboardKey::DOWN;
    case GLFW_KEY_LEFT_SHIFT: return KeyboardKey::LEFT_SHIFT;
    case GLFW_KEY_SPACE: return KeyboardKey::SPACE;
    case GLFW_KEY_ESCAPE: return KeyboardKey::ESCAPE;
    case GLFW_KEY_C: return KeyboardKey::C;
    case GLFW_KEY_U: return KeyboardKey::U;
    default:
      throw std::logic_error("Key not yet implemented");
  }
}

MouseButton Window::getMouseButton(int button) {
  switch (button) {
    case GLFW_MOUSE_BUTTON_LEFT: return MouseButton::LEFT;
    case GLFW_MOUSE_BUTTON_RIGHT: return MouseButton::RIGHT;
    default:
      throw std::logic_error("Button not yet implemented");
 
  }
}

void Window::pollEvents() const { glfwPollEvents(); }
void Window::waitEvents() const { glfwWaitEvents(); }

void Window::setMouseMode(MouseMode mouseMode) {
  switch (mouseMode) { 
  case MouseMode::NORMAL:
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    return;
  case MouseMode::HIDDEN:
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    return;
  case MouseMode::DISABLED:
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    return;
  default:
    throw std::logic_error("Mouse mode not yet implemented");
  }
}


void Window::setRawMouseMotion(bool enabled) {
  if (!enabled) {
    glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
    return;
  }

  if (!glfwRawMouseMotionSupported()) {
    return;
    // TODO: Throw error
    //throw std::exception("Raw mouse motion is not supported");
  }


  glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
}


bool Window::isRawMouseMotionSupported() {
  return glfwRawMouseMotionSupported();
}

void Window::closeCallback(GLFWwindow* window) {
  Window* handler = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
  if (handler) {
    for (auto listener : handler->_closeListeners) {
      listener();
    }
  }
}

}
