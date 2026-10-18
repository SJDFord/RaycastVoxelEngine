#pragma once

#include <glm/glm.hpp>

namespace engine {

struct SimplePushConstantData {
  glm::mat4 modelMatrix{1.f};
  glm::mat4 normalMatrix{1.f};
};

}