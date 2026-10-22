#pragma once

#include "../graphics/vulkan/game_object.hpp"
#include "../graphics/vulkan/window.hpp"
#include <glm/glm.hpp>
#include <iostream>


namespace engine {
class FpsMovementController {
 public:
  FpsMovementController(Window& window);
  void updateView(Window& window, float dt, GameObject& gameObject);
  float moveSpeed{10.f};
  float lookSpeed{20.0f};
 private: 
  glm::vec3 _movementDirection{0, 0, 0};
  glm::vec2 _mousePos{0, 0};
  glm::vec2 _lastMousePos{0, 0};
  float yaw{-90.0f};
  float pitch{0.0f};
  void setMouseCapture(Window& window, bool capture);

};
}  // namespace lve
