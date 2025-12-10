#pragma once

#include "game_object.hpp"
#include <lve_camera.hpp>

#define MAX_LIGHTS 10

struct PointLight {
  glm::vec4 position{};  // ignore w
  glm::vec4 color{};     // w is intensity
};

struct GlobalUbo {
  glm::mat4 projection{1.f};
  glm::mat4 view{1.f};
  glm::mat4 inverseView{1.f};
  glm::vec4 ambientLightColor{1.f, 1.f, 1.f, .02f};  // w is intensity
  PointLight pointLights[MAX_LIGHTS];
  int numLights;
};

struct FrameInfo {
  int frameIndex;
  float frameTime;
  const vk::raii::CommandBuffer& commandBuffer;
  lve::LveCamera &camera;
  vk::DescriptorSet globalDescriptorSet;
  GameObject::Map &gameObjects;
};
