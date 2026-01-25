#pragma once

#include "lve_game_object.hpp"
#include "lve_window.hpp"
#include <glm/glm.hpp>
#include <iostream>


namespace lve {
class FpsMovementController {
 public:
  struct KeyMappings {
    int moveLeft = GLFW_KEY_A;
    int moveRight = GLFW_KEY_D;
    int moveForward = GLFW_KEY_W;
    int moveBackward = GLFW_KEY_S;
    int moveUp = GLFW_KEY_SPACE;
    int moveDown = GLFW_KEY_LEFT_SHIFT;
    int lookLeft = GLFW_KEY_LEFT;
    int lookRight = GLFW_KEY_RIGHT;
    int lookUp = GLFW_KEY_UP;
    int lookDown = GLFW_KEY_DOWN;
    int enableMouseCapture = GLFW_KEY_C;
    int disableMouseCapture = GLFW_KEY_U;
  };
  FpsMovementController(GLFWwindow* window);
  void updateView(GLFWwindow* window, float dt, LveGameObject& gameObject);

  KeyMappings keys{};
  float moveSpeed{10.f};
  float lookSpeed{20.0f};
 private: 
  glm::vec2 lastMousePos{0, 0};
  float yaw{-90.0f};
  float pitch{0.0f};
  void getMousePos(GLFWwindow* window, glm::vec2& result);
  void setMouseCapture(GLFWwindow* window, bool capture);
};
}  // namespace lve
