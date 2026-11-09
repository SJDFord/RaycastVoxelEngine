#pragma once

#include <glm/glm.hpp>

namespace engine {

struct SimplePushConstantData {
  glm::mat4 modelMatrix{1.f};
  glm::mat4 normalMatrix{1.f};
  int textureIndex;
};

struct PerModelPushConstantData {
  int textureIndex;
};

struct DemoData
{
	vk::Sampler						sampler;
	vk::DescriptorImageInfo			descriptorImageInfos[8];
};

}