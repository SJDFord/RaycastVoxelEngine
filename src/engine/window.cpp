#include "window.hpp"

// std
#include <stdexcept>

namespace engine {

Window::Window(const vk::Instance& instance, std::string name, int w, int h) : windowName{name}, width{w}, height{h} {
  initWindow(instance);
}

Window::~Window() {
  glfwDestroyWindow(window);
  glfwTerminate();
}

void Window::initWindow(const vk::Instance& instance) {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
  glfwSetWindowUserPointer(window, this);
  glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);


  VkSurfaceKHR surface;
  VkResult     err = glfwCreateWindowSurface( instance, window, nullptr, &surface );
  if ( err != VK_SUCCESS )
    throw std::runtime_error( "Failed to create window!" );
  _surface = vk::SurfaceKHR( surface );
  _extent = vk::Extent2D( static_cast<uint32_t>(width), static_cast<uint32_t>(height) );
}

std::vector<const char *>  Window::getRequiredExtensions() {
  uint32_t glfwExtensionCount = 0;
  const char **glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
  return extensions;
}

const vk::SurfaceKHR& Window::getSurface() {
    return _surface;
}

void Window::framebufferResizeCallback(GLFWwindow *window, int width, int height) {
  auto w = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
  w->framebufferResized = true;
  w->width = width;
  w->height = height;
}

void Window::pollEvents() { glfwPollEvents(); }
void Window::waitEvents() { glfwWaitEvents(); }

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

void Window::getMousePosition(glm::vec2 &result) {
  double x, y;
  glfwGetCursorPos(window, &x, &y);
  result.x = x;
  result.y = y;
}

bool Window::isMouseButtonPressed(MouseButton mouseButton) {
  int state = GLFW_RELEASE;
  switch (mouseButton) {
    case MouseButton::LEFT:
        state = glfwGetKey(window, GLFW_MOUSE_BUTTON_LEFT);
        break;
    case MouseButton::RIGHT:
        state = glfwGetKey(window, GLFW_MOUSE_BUTTON_RIGHT);
        break;
    default:
        throw std::logic_error("Button not yet implemented");
  }

  return state == GLFW_PRESS;
}

bool Window::isKeyPressed(KeyboardKey key) {
  // TODO: Generate this code and the Keyboardkey enum
  int state = GLFW_RELEASE;
  switch (key) { 
  case KeyboardKey::A:
    state = glfwGetKey(window, GLFW_KEY_A);
    break;
  case KeyboardKey::D:
    state = glfwGetKey(window, GLFW_KEY_D);
    break;
  case KeyboardKey::S:
    state = glfwGetKey(window, GLFW_KEY_S);
    break;
  case KeyboardKey::W:
    state = glfwGetKey(window, GLFW_KEY_W);
    break;
  case KeyboardKey::LEFT:
    state = glfwGetKey(window, GLFW_KEY_LEFT);
    break;
  case KeyboardKey::RIGHT:
    state = glfwGetKey(window, GLFW_KEY_RIGHT);
    break;
  case KeyboardKey::UP:
    state = glfwGetKey(window, GLFW_KEY_UP);
    break;
  case KeyboardKey::DOWN:
    state = glfwGetKey(window, GLFW_KEY_DOWN);
    break;
  case KeyboardKey::LEFT_SHIFT:
    state = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT);
    break;
  case KeyboardKey::SPACE:
    state = glfwGetKey(window, GLFW_KEY_SPACE);
    break;
  case KeyboardKey::ESCAPE:
    state = glfwGetKey(window, GLFW_KEY_ESCAPE);
    break;
  default:
    throw std::logic_error("Key not yet implemented");
  }

  return state == GLFW_PRESS;
}

void Window::close() { glfwSetWindowShouldClose(window, 1); }

}
