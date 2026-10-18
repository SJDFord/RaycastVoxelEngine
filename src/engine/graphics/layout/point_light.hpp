#pragma once

#include <glm/glm.hpp>

namespace engine {

struct PointLightPushConstants {
  glm::vec4 position{};
  glm::vec4 color{};
  float radius;
};

}