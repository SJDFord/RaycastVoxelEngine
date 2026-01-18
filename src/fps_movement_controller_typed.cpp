#include "fps_movement_controller_typed.hpp"

// std
#include <limits>

FpsMovementControllerTyped::FpsMovementControllerTyped(std::shared_ptr<Window> window) { 
    window->setMouseMode(MouseMode::DISABLED);
    window->setRawMouseMotion(true);
    window->getMousePosition(lastMousePos);
}

void FpsMovementControllerTyped::updateView(
    std::shared_ptr<Window> window, float dt, GameObject& gameObject) {
  glm::vec2 mousePos;
  window->getMousePosition(mousePos);

  glm::vec2 mousePosDelta = mousePos - lastMousePos;
  lastMousePos = mousePos;
  glm::vec3 rotate{0};
  rotate.x -= mousePosDelta.y;
  rotate.y += mousePosDelta.x;
  if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
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
  if (window->isKeyPressed(KeyboardKey::W)) {
        moveDir += forwardDir;
  } else if (window->isKeyPressed(KeyboardKey::S)) {
        moveDir -= forwardDir;
  } else if (window->isKeyPressed(KeyboardKey::D)) {
        //std::cout << "Walking Right" << std::endl;
        moveDir += rightDir;
  } else if (window->isKeyPressed(KeyboardKey::A)) {
        //std::cout << "Walking Left" << std::endl;
        moveDir -= rightDir;
  } else if (window->isKeyPressed(KeyboardKey::SPACE)) {
        moveDir += upDir;
  } else if (window->isKeyPressed(KeyboardKey::LEFT_SHIFT)) {
        moveDir -= upDir;
  }

  if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
    gameObject.transform.translation += moveSpeed * dt * glm::normalize(moveDir);
  }
}
