#include "simple_render_system_typed.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <stdexcept>

struct SimplePushConstantData {
  glm::mat4 modelMatrix{1.f};
  glm::mat4 normalMatrix{1.f};
};

SimpleRenderSystemTyped::SimpleRenderSystemTyped(
    Device& device, const vk::raii::RenderPass& renderPass, const vk::raii::DescriptorSetLayout& globalSetLayout)
    : device{device} {
  createPipelineLayout(globalSetLayout);
  createPipeline(renderPass);
}

SimpleRenderSystemTyped::~SimpleRenderSystemTyped() {
  //device.destroyPipelineLayout(pipelineLayout);
}

void SimpleRenderSystemTyped::createPipelineLayout(const vk::raii::DescriptorSetLayout& globalSetLayout) {
  vk::PushConstantRange pushConstantRange = vk::PushConstantRange()
      .setStageFlags(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment)
      .setOffset(0)
      .setSize(sizeof(SimplePushConstantData));

  vk::PipelineLayoutCreateInfo pipelineLayoutInfo = vk::PipelineLayoutCreateInfo(
      vk::PipelineLayoutCreateFlags(),
      *globalSetLayout,
      pushConstantRange);

  pipelineLayout = device.device().createPipelineLayout(pipelineLayoutInfo);
}

void SimpleRenderSystemTyped::createPipeline(const vk::raii::RenderPass& renderPass) {
  // TODO
    //assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

  PipelineConfigInfo pipelineConfig = PipelineConfigInfo(pipelineLayout, renderPass);
  Pipeline::defaultPipelineConfigInfo(pipelineConfig);
  pipeline = std::make_unique<Pipeline>(
      device,
      "shaders/simple_shader.vert.spv",
      "shaders/simple_shader.frag.spv",
      pipelineConfig);
}

void SimpleRenderSystemTyped::renderGameObjects(FrameInfo& frameInfo) {
  pipeline->bind(frameInfo.commandBuffer);
  frameInfo.commandBuffer.bindDescriptorSets(
      vk::PipelineBindPoint::eGraphics,
      pipelineLayout,
      0,
      frameInfo.globalDescriptorSet,
      {}
  );

  for (auto& kv : frameInfo.gameObjects) {
    auto& obj = kv.second;
    if (obj.model == nullptr) continue;
    SimplePushConstantData push{};
    push.modelMatrix = obj.transform.mat4();
    push.normalMatrix = obj.transform.normalMatrix();

    vk::ArrayProxy<const SimplePushConstantData> values = push;
    frameInfo.commandBuffer.pushConstants(
        pipelineLayout,
        vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
        0,
        values);
    obj.model->bind(frameInfo.commandBuffer);
    obj.model->draw(frameInfo.commandBuffer);
  }
}