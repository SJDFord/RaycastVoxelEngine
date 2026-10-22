#include "fps_movement_controller.hpp"

// std
#include <print>
#include <limits>
#include <algorithm>

namespace engine {

FpsMovementController::FpsMovementController(Window& window) {
  //setMouseCapture(window, true);
  window.addKeyListener([&] (KeyboardKey key, bool isPressed, vk::Flags<ModifierFlagBits> modifierFlags) {
    if (key == KeyboardKey::A) _movementDirection.x = isPressed ? -1 : std::max(_movementDirection.x, 0.f);
    if (key == KeyboardKey::D) _movementDirection.x = isPressed ? 1 : std::min(_movementDirection.x, 0.f);
    if (key == KeyboardKey::W) _movementDirection.y = isPressed ? 1 : std::min(_movementDirection.y, 0.f);
    if (key == KeyboardKey::S) _movementDirection.y = isPressed ? -1 : std::max(_movementDirection.y, 0.f);
    if (key == KeyboardKey::SPACE) _movementDirection.z = isPressed ? 1 : std::min(_movementDirection.z, 0.f);
    if (key == KeyboardKey::LEFT_SHIFT) _movementDirection.z = isPressed ? -1 : std::max(_movementDirection.z, 0.f);
    if (key == KeyboardKey::C && isPressed) setMouseCapture(window, true);
    if (key == KeyboardKey::U && isPressed) setMouseCapture(window, false);
  });

  window.addCursorPositionListener([&] (glm::dvec2 position) {
    _mousePos.x = position.x;
    _mousePos.y = position.y;
  });
}

void FpsMovementController::updateView(
    Window& window, float dt, GameObject& gameObject) {
  glm::vec2 mousePosDelta = _mousePos - _lastMousePos;
  _lastMousePos = _mousePos;
  glm::vec3 rotate{0};
  rotate.x -= mousePosDelta.y;
  rotate.y += mousePosDelta.x;
  if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
  	//std::cout << "RotX: " << rotate.x << ", RotY: " << rotate.y << std::endl;
        gameObject.transform.rotation += lookSpeed * dt * glm::normalize(rotate);
  }
  
  // limit pitch values between about +/- 85ish degrees and minimise the rotation value (e.g. 10 degrees instead of 370 degrees)
  gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
  gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());

  float yaw = gameObject.transform.rotation.y;
  const glm::vec3 forwardDir{sin(yaw), 0.f, cos(yaw)};
  const glm::vec3 rightDir{forwardDir.z, 0.f, -forwardDir.x};
  const glm::vec3 upDir{0.f, -1.f, 0.f};
  glm::vec3 moveDir{0.f};
  moveDir += forwardDir * _movementDirection.y;
  moveDir += rightDir * _movementDirection.x;
  moveDir += upDir * _movementDirection.z;

  if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
    gameObject.transform.translation += moveSpeed * dt * glm::normalize(moveDir);
  }
}
  
  void FpsMovementController::setMouseCapture(Window& window, bool capture) {
    if (capture) {
        window.setMouseMode(MouseMode::DISABLED);
        
        if (window.isRawMouseMotionSupported()) {
          window.setRawMouseMotion(true);
        }
        
        return;
    }

    window.setMouseMode(MouseMode::NORMAL);
		return;
  }

}  // namespace lve
