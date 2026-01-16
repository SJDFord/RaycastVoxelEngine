#include "point_light_system_typed.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <map>
#include <stdexcept>

struct PointLightPushConstants {
  glm::vec4 position{};
  glm::vec4 color{};
  float radius;
};

PointLightSystemTyped::PointLightSystemTyped(
    std::shared_ptr<Device> device,
    const vk::raii::RenderPass& renderPass,
    const vk::raii::DescriptorSetLayout& globalSetLayout)
    : device{device} {
  createPipelineLayout(globalSetLayout);
  createPipeline(renderPass);
}

PointLightSystemTyped::~PointLightSystemTyped() {
  //device.destroyPipelineLayout(pipelineLayout);
}

void PointLightSystemTyped::createPipelineLayout(const vk::raii::DescriptorSetLayout& globalSetLayout) {
  vk::PushConstantRange pushConstantRange = vk::PushConstantRange()
      .setStageFlags(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment)
      .setOffset(0)
      .setSize(sizeof(PointLightPushConstants));

  vk::PipelineLayoutCreateInfo pipelineLayoutInfo = vk::PipelineLayoutCreateInfo(
      vk::PipelineLayoutCreateFlags(),
      *globalSetLayout,
      pushConstantRange);

  pipelineLayout = device->device().createPipelineLayout(pipelineLayoutInfo);
}

void PointLightSystemTyped::createPipeline(const vk::raii::RenderPass& renderPass) {
  //assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

  PipelineConfigInfo pipelineConfig = PipelineConfigInfo(pipelineLayout, renderPass);
  Pipeline::defaultPipelineConfigInfo(pipelineConfig);
  Pipeline::enableAlphaBlending(pipelineConfig);
  pipeline = std::make_unique<Pipeline>(
      device,
      "shaders/point_light.vert.spv",
      "shaders/point_light.frag.spv",
      pipelineConfig);
}

void PointLightSystemTyped::update(FrameInfo& frameInfo, GlobalUbo& ubo) {
  auto rotateLight = glm::rotate(glm::mat4(1.f), 0.5f * frameInfo.frameTime, {0.f, -1.f, 0.f});
  int lightIndex = 0;
  for (auto& kv : frameInfo.gameObjects) {
    auto& obj = kv.second;
    if (obj.pointLight == nullptr) continue;

    assert(lightIndex < MAX_LIGHTS && "Point lights exceed maximum specified");

    // update light position
    obj.transform.translation = glm::vec3(rotateLight * glm::vec4(obj.transform.translation, 1.f));

    // copy light to ubo
    ubo.pointLights[lightIndex].position = glm::vec4(obj.transform.translation, 1.f);
    ubo.pointLights[lightIndex].color = glm::vec4(obj.color, obj.pointLight->lightIntensity);

    lightIndex += 1;
  }
  ubo.numLights = lightIndex;
}

void PointLightSystemTyped::render(FrameInfo& frameInfo) {
  // sort lights
  std::map<float, GameObject::id_t> sorted;
  for (auto& kv : frameInfo.gameObjects) {
    auto& obj = kv.second;
    if (obj.pointLight == nullptr) continue;

    // calculate distance
    auto offset = frameInfo.camera.getPosition() - obj.transform.translation;
    float disSquared = glm::dot(offset, offset);
    sorted[disSquared] = obj.getId();
  }

  pipeline->bind(frameInfo.commandBuffer);

  frameInfo.commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics,
        pipelineLayout,
        0,
        frameInfo.globalDescriptorSet,
        {});

  // iterate through sorted lights in reverse order
  for (auto it = sorted.rbegin(); it != sorted.rend(); ++it) {
    // use game obj id to find light object
    auto& obj = frameInfo.gameObjects.at(it->second);

    PointLightPushConstants push{};
    push.position = glm::vec4(obj.transform.translation, 1.f);
    push.color = glm::vec4(obj.color, obj.pointLight->lightIntensity);
    push.radius = obj.transform.scale.x;

    vk::ArrayProxy<const PointLightPushConstants> values = push;
    frameInfo.commandBuffer.pushConstants(
        pipelineLayout,
        vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
        0,
        values);

    frameInfo.commandBuffer.draw(6, 1, 0, 0);
  }
}
