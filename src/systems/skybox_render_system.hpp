#pragma once

#include "../engine/graphics/vulkan/utils.hpp"
#include "../engine/graphics/vulkan/builders/pipeline_builder.hpp"
#include "../engine/graphics/vulkan/builders/shader_module_builder.hpp"
#include "../engine/graphics/layout/global.hpp"
#include "../engine/graphics/layout/simple.hpp"
#include "../engine/graphics/vulkan/frame_info.hpp"

// std
#include <memory>
#include <vector>

namespace lve {
class SkyboxRenderSystem {
 public:
  SkyboxRenderSystem(
      vk::Device device, vk::RenderPass renderPass, vk::Format colorFormat, vk::Format depthFormat, vk::DescriptorSetLayout globalSetLayout);
  ~SkyboxRenderSystem();

  SkyboxRenderSystem(const SkyboxRenderSystem &) = delete;
  SkyboxRenderSystem &operator=(const SkyboxRenderSystem &) = delete;

  void renderGameObjects(engine::FrameInfo &frameInfo) const;

 private:
  void createPipelineLayout(vk::DescriptorSetLayout globalSetLayout);
  void createPipeline(vk::RenderPass renderPass, vk::Format colorFormat, vk::Format depthFormat);

  vk::Device _device;
  vk::Pipeline _pipeline;
  vk::PipelineLayout _pipelineLayout;
};
}  // namespace lve
