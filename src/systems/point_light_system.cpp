#include "point_light_system.hpp"

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

namespace lve {

PointLightSystem::PointLightSystem(
    vk::Device device, vk::RenderPass renderPass, vk::DescriptorSetLayout globalSetLayout)
    : _device{device} {
  createPipelineLayout(globalSetLayout);
  createPipeline(renderPass);
}

PointLightSystem::~PointLightSystem() {
  _device.destroyPipelineLayout(_pipelineLayout);
}

void PointLightSystem::createPipelineLayout(vk::DescriptorSetLayout globalSetLayout) {
  vk::PushConstantRange pushConstantRange = vk::PushConstantRange(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment)
    .setOffset(0)
    .setSize(sizeof(engine::PointLightPushConstants));

  std::vector<vk::DescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

  vk::PipelineLayoutCreateInfo pipelineLayoutInfo = vk::PipelineLayoutCreateInfo()
    .setSetLayouts(descriptorSetLayouts)
    .setPushConstantRanges(pushConstantRange);

  _pipelineLayout = _device.createPipelineLayout(pipelineLayoutInfo);
}

void PointLightSystem::createPipeline(vk::RenderPass renderPass) {
  std::string vertShaderGlsl = engine::readFileString("../shaders/point_light.vert");
  std::string fragShaderGlsl = engine::readFileString("../shaders/point_light.frag");
  vk::ShaderModule vertexShaderModule =
      engine::ShaderModuleBuilder(_device, vk::ShaderStageFlagBits::eVertex, vertShaderGlsl)
          .build();
  vk::ShaderModule fragmentShaderModule =
      engine::ShaderModuleBuilder(_device, vk::ShaderStageFlagBits::eFragment, fragShaderGlsl)
          .build();

  _pipeline = engine::PipelineBuilder(_device, _pipelineLayout)
              .addShaderModule(vertexShaderModule, vk::ShaderStageFlagBits::eVertex)
              .addShaderModule(fragmentShaderModule, vk::ShaderStageFlagBits::eFragment)
              .setRenderPass(renderPass)
              .setBindingDescriptions(engine::Vertex::getBindingDescriptions())
              .setAttributeDescriptions(engine::Vertex::getAttributeDescriptions())
              .setBlending(true)
              .build();
}

void PointLightSystem::update(engine::FrameInfo& frameInfo, engine::GlobalUbo& ubo) {
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

void PointLightSystem::render(engine::FrameInfo& frameInfo) {
  // sort lights
  std::map<float, engine::GameObject::id_t> sorted;
  for (auto& kv : frameInfo.gameObjects) {
    auto& obj = kv.second;
    if (obj.pointLight == nullptr) continue;

    // calculate distance
    auto offset = frameInfo.camera.getPosition() - obj.transform.translation;
    float disSquared = glm::dot(offset, offset);
    sorted[disSquared] = obj.getId();
  }

  frameInfo.commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _pipeline);
  frameInfo.commandBuffer.bindDescriptorSets(
    vk::PipelineBindPoint::eGraphics, 
    _pipelineLayout, 
    0,
    frameInfo.globalDescriptorSet, 
    {}
  );
  

  // iterate through sorted lights in reverse order
  for (auto it = sorted.rbegin(); it != sorted.rend(); ++it) {
    // use game obj id to find light object
    auto& obj = frameInfo.gameObjects.at(it->second);

    engine::PointLightPushConstants push{};
    push.position = glm::vec4(obj.transform.translation, 1.f);
    push.color = glm::vec4(obj.color, obj.pointLight->lightIntensity);
    push.radius = obj.transform.scale.x;
    
    frameInfo.commandBuffer.pushConstants(
      _pipelineLayout, 
      vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 
      0,
      sizeof(engine::PointLightPushConstants),
      &push);
    
    frameInfo.commandBuffer.draw(6, 1, 0, 0);

    //vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
  }
}

}  // namespace lve
