#include "fps_movement_controller.hpp"

// std
#include <limits>

namespace lve {

FpsMovementController::FpsMovementController(GLFWwindow* window) { 
    setMouseCapture(window, true);
}

void FpsMovementController::updateView(
    GLFWwindow* window, float dt, LveGameObject& gameObject) {
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
  if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS) moveDir += forwardDir;
  if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS) moveDir -= forwardDir;
  if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS) moveDir += rightDir;
  if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS) moveDir -= rightDir;
  if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS) moveDir += upDir;
  if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS) moveDir -= upDir;
  if (glfwGetKey(window, keys.enableMouseCapture) == GLFW_PRESS) {
  	setMouseCapture(window, true);
  }
  if (glfwGetKey(window, keys.disableMouseCapture) == GLFW_PRESS) {
  	setMouseCapture(window, false);
  }
	
  if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
    gameObject.transform.translation += moveSpeed * dt * glm::normalize(moveDir);
  }
}

  void FpsMovementController::getMousePos(GLFWwindow* window, glm::vec2& result) { 
      double x, y;
      glfwGetCursorPos(window, &x, &y);
      //std::cout << "[RAW] RotX: " << x << ", RotY: " << y << std::endl;
      result.x = x;
      result.y = y;
  }
  
  
  void FpsMovementController::setMouseCapture(GLFWwindow* window, bool capture) {
	if (capture) {
	    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	    
	    if (glfwRawMouseMotionSupported()) {
		glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
		//std::cout << "Enabling raw mouse motion" << std::endl;
	    }
	    
	    getMousePos(window, lastMousePos);
	    return;
	}

		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		
		//glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
		return;
  }

}  // namespace lve
