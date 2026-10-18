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
class SimpleRenderSystem {
 public:
  SimpleRenderSystem(
      vk::Device device, vk::RenderPass renderPass, vk::DescriptorSetLayout globalSetLayout);
  ~SimpleRenderSystem();

  SimpleRenderSystem(const SimpleRenderSystem &) = delete;
  SimpleRenderSystem &operator=(const SimpleRenderSystem &) = delete;

  void renderGameObjects(engine::FrameInfo &frameInfo) const;

 private:
  void createPipelineLayout(vk::DescriptorSetLayout globalSetLayout);
  void createPipeline(vk::RenderPass renderPass);

  vk::Device _device;
  vk::Pipeline _pipeline;
  vk::PipelineLayout _pipelineLayout;
};
}  // namespace lve
