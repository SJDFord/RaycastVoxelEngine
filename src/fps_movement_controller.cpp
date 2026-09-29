#include "fps_movement_controller.hpp"

// std
#include <limits>

namespace lve {

FpsMovementController::FpsMovementController(engine::Window &window) { 
    setMouseCapture(window, true);
}

void FpsMovementController::updateView(
    engine::Window &window, float dt, LveGameObject& gameObject) {
  glm::vec2 mousePos;
  getMousePos(window, mousePos);
  glm::vec2 mousePosDelta = mousePos - lastMousePos;
  lastMousePos = mousePos;
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
  if (window.isKeyPressed(engine::KeyboardKey::W)) moveDir += forwardDir;
  if (window.isKeyPressed(engine::KeyboardKey::S)) moveDir -= forwardDir;
  if (window.isKeyPressed(engine::KeyboardKey::D)) moveDir += rightDir;
  if (window.isKeyPressed(engine::KeyboardKey::A)) moveDir -= rightDir;
  if (window.isKeyPressed(engine::KeyboardKey::SPACE)) moveDir += upDir;
  if (window.isKeyPressed(engine::KeyboardKey::LEFT_SHIFT)) moveDir -= upDir;
  if (window.isKeyPressed(engine::KeyboardKey::C)) {
  	setMouseCapture(window, true);
  }
  if (window.isKeyPressed(engine::KeyboardKey::U)) {
  	setMouseCapture(window, false);
  }
	
  if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
    gameObject.transform.translation += moveSpeed * dt * glm::normalize(moveDir);
  }
}

  void FpsMovementController::getMousePos(engine::Window &window, glm::vec2& result) { 
    window.getMousePosition(result);
  }
  
  
  void FpsMovementController::setMouseCapture(engine::Window &window, bool capture) {
    if (capture) {
        window.setMouseMode(engine::MouseMode::DISABLED);
        
        if (window.isRawMouseMotionSupported()) {
          window.setRawMouseMotion(true);
        }
        
        getMousePos(window, lastMousePos);
        return;
    }

    window.setMouseMode(engine::MouseMode::NORMAL);
		return;
  }

}  // namespace lve
