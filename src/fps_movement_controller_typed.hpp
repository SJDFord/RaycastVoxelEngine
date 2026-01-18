#pragma once

#include "game_object.hpp"
#include "window.hpp"
#include <glm/glm.hpp>

class FpsMovementControllerTyped {
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
  };
  FpsMovementControllerTyped(std::shared_ptr<Window> window);
  void updateView(std::shared_ptr<Window> window, float dt, GameObject& gameObject);

  KeyMappings keys{};
  float moveSpeed{10.f};
  float lookSpeed{2.0f};
 private: 
  glm::vec2 lastMousePos{0, 0};
  float yaw{-90.0f};
  float pitch{0.0f};
};