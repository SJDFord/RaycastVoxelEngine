#include "simple_render_system.hpp"
#include "../engine/utils.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <stdexcept>

namespace lve {

SimpleRenderSystem::SimpleRenderSystem(
    vk::Device device, vk::RenderPass renderPass, vk::DescriptorSetLayout globalSetLayout)
    : _device{device} {
  createPipelineLayout(globalSetLayout);
  createPipeline(renderPass);
}

SimpleRenderSystem::~SimpleRenderSystem() {
  _device.destroyPipelineLayout(pipelineLayout);
}

void SimpleRenderSystem::createPipelineLayout(vk::DescriptorSetLayout globalSetLayout) {
  vk::PushConstantRange pushConstantRange = vk::PushConstantRange(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment)
    .setOffset(0)
    .setSize(sizeof(engine::SimplePushConstantData));

  std::vector<vk::DescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

  vk::PipelineLayoutCreateInfo pipelineLayoutInfo = vk::PipelineLayoutCreateInfo()
    .setSetLayouts(descriptorSetLayouts)
    .setPushConstantRanges(pushConstantRange);

  pipelineLayout = _device.createPipelineLayout(pipelineLayoutInfo);
}

void SimpleRenderSystem::createPipeline(vk::RenderPass renderPass) {
  std::string vertShaderGlsl = engine::readFileString("../shaders/simple_shader.vert");
  std::string fragShaderGlsl = engine::readFileString("../shaders/simple_shader.frag");
  vk::ShaderModule vertexShaderModule =
      engine::ShaderModuleBuilder(_device, vk::ShaderStageFlagBits::eVertex, vertShaderGlsl)
          .build();
  vk::ShaderModule fragmentShaderModule =
      engine::ShaderModuleBuilder(_device, vk::ShaderStageFlagBits::eFragment, fragShaderGlsl)
          .build();

  _pipeline = engine::PipelineBuilder(_device, pipelineLayout)
              .addShaderModule(vertexShaderModule, vk::ShaderStageFlagBits::eVertex)
              .addShaderModule(fragmentShaderModule, vk::ShaderStageFlagBits::eFragment)
              .setRenderPass(renderPass)
              .setBindingDescriptions(engine::Vertex::getBindingDescriptions())
              .setAttributeDescriptions(engine::Vertex::getAttributeDescriptions())
              .build();
}

void SimpleRenderSystem::renderGameObjects(engine::FrameInfo& frameInfo) {
  frameInfo.commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _pipeline);
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
    engine::SimplePushConstantData push{};
    push.modelMatrix = obj.transform.mat4();
    push.normalMatrix = obj.transform.normalMatrix();

    frameInfo.commandBuffer.pushConstants(
      pipelineLayout, 
      vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 
      0,
      sizeof(engine::SimplePushConstantData),
      &push);

    obj.model->bind(frameInfo.commandBuffer);
    obj.model->draw(frameInfo.commandBuffer);
  }
}

}  // namespace lve
