#include "skybox_render_system.hpp"
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

SkyboxRenderSystem::SkyboxRenderSystem(
    vk::Device device, vk::RenderPass renderPass, vk::Format colorFormat, vk::Format depthFormat, vk::DescriptorSetLayout globalSetLayout)
    : _device{device} {
  createPipelineLayout(globalSetLayout);
  createPipeline(renderPass, colorFormat, depthFormat);
}

SkyboxRenderSystem::~SkyboxRenderSystem() {
  _device.destroyPipelineLayout(_pipelineLayout);
}

void SkyboxRenderSystem::createPipelineLayout(vk::DescriptorSetLayout globalSetLayout) {
  vk::PushConstantRange pushConstantRange = vk::PushConstantRange(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment)
    .setOffset(0)
    .setSize(sizeof(engine::SimplePushConstantData));

  std::vector<vk::DescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

  vk::PipelineLayoutCreateInfo pipelineLayoutInfo = vk::PipelineLayoutCreateInfo()
    .setSetLayouts(descriptorSetLayouts)
    .setPushConstantRanges(pushConstantRange);

  _pipelineLayout = _device.createPipelineLayout(pipelineLayoutInfo);
}

void SkyboxRenderSystem::createPipeline(vk::RenderPass renderPass, vk::Format colorFormat, vk::Format depthFormat) {
  std::string vertShaderGlsl = engine::readFileString("../shaders/skybox.vert");
  std::string fragShaderGlsl = engine::readFileString("../shaders/skybox.frag");
  vk::ShaderModule vertexShaderModule =
      engine::ShaderModuleBuilder(_device, vk::ShaderStageFlagBits::eVertex, vertShaderGlsl)
          .build();
  vk::ShaderModule fragmentShaderModule =
      engine::ShaderModuleBuilder(_device, vk::ShaderStageFlagBits::eFragment, fragShaderGlsl)
          .build();

  auto rasterizationInfo = vk::PipelineRasterizationStateCreateInfo()
    .setDepthClampEnable(vk::False)
    .setRasterizerDiscardEnable(vk::False)
    .setPolygonMode(vk::PolygonMode::eFill)
    .setLineWidth(1.0f)
    .setCullMode(vk::CullModeFlagBits::eFront)
    .setFrontFace(vk::FrontFace::eCounterClockwise)
    .setDepthBiasEnable(vk::False)
    .setDepthBiasConstantFactor(0.0f)
    .setDepthBiasClamp(0.0f)
    .setDepthBiasSlopeFactor(0.0f);

  auto back = vk::StencilOpState().setCompareOp(vk::CompareOp::eAlways);
  auto depthStencilInfo = vk::PipelineDepthStencilStateCreateInfo()
    .setDepthTestEnable(vk::False)
    .setDepthWriteEnable(vk::False)
    .setDepthCompareOp(vk::CompareOp::eLessOrEqual)
    .setDepthBoundsTestEnable(vk::False)
    .setMinDepthBounds(0.0f)
    .setMaxDepthBounds(1.0f)
    .setStencilTestEnable(vk::False)
    .setFront({})
    .setBack(back);

  _pipeline = engine::PipelineBuilder(_device, _pipelineLayout)
              .addShaderModule(vertexShaderModule, vk::ShaderStageFlagBits::eVertex)
              .addShaderModule(fragmentShaderModule, vk::ShaderStageFlagBits::eFragment)
              .setRenderPass(renderPass)
              .setBindingDescriptions(engine::Vertex::getBindingDescriptions())
              .setAttributeDescriptions(engine::Vertex::getAttributeDescriptions())
              .setRasterizationInfo(rasterizationInfo)
              .setDepthStencilInfo(depthStencilInfo)
              .build();
}

void SkyboxRenderSystem::renderGameObjects(engine::FrameInfo& frameInfo) const {
  frameInfo.commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _pipeline);
  frameInfo.commandBuffer.bindDescriptorSets(
    vk::PipelineBindPoint::eGraphics, 
    _pipelineLayout, 
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
      _pipelineLayout, 
      vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 
      0,
      sizeof(engine::SimplePushConstantData),
      &push);

    obj.model->bind(frameInfo.commandBuffer);
    obj.model->draw(frameInfo.commandBuffer);
  }
}

}  // namespace lve
