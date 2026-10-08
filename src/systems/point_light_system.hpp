#pragma once

#include "../engine/utils.hpp"

#include "../engine/pipeline_builder.hpp"
#include "../engine/shader_module_builder.hpp"
#include "lve_device.hpp"
#include "lve_pipeline.hpp"

// std
#include <memory>
#include <vector>

namespace lve {
class PointLightSystem {
 public:
  PointLightSystem(
      vk::Device device, vk::RenderPass renderPass, vk::DescriptorSetLayout globalSetLayout);
  ~PointLightSystem();

  PointLightSystem(const PointLightSystem &) = delete;
  PointLightSystem &operator=(const PointLightSystem &) = delete;

  void update(engine::FrameInfo &frameInfo, engine::GlobalUbo &ubo);
  void render(engine::FrameInfo &frameInfo);

 private:
  void createPipelineLayout(vk::DescriptorSetLayout globalSetLayout);
  void createPipeline(vk::RenderPass renderPass);

  vk::Device _device;

  vk::Pipeline _pipeline;
  vk::PipelineLayout pipelineLayout;
};
}  // namespace lve
