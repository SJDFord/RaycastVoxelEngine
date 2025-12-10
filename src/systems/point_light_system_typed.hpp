#pragma once

#include "lve_camera.hpp"
#include "device.hpp"
#include "frame_info.hpp"
#include "game_object.hpp"
#include "pipeline.hpp"

// std
#include <memory>
#include <vector>

class PointLightSystemTyped {
 public:
  PointLightSystemTyped(
      Device &device,
      const vk::raii::RenderPass& renderPass,
      const vk::raii::DescriptorSetLayout& globalSetLayout);
  ~PointLightSystemTyped();

  PointLightSystemTyped(const PointLightSystemTyped &) = delete;
  PointLightSystemTyped &operator=(const PointLightSystemTyped &) = delete;

  void update(FrameInfo &frameInfo, GlobalUbo &ubo);
  void render(FrameInfo &frameInfo);

 private:
  void createPipelineLayout(const vk::raii::DescriptorSetLayout& globalSetLayout);
  void createPipeline(const vk::raii::RenderPass& renderPass);

  Device &device;

  std::unique_ptr<Pipeline> pipeline;
  vk::raii::PipelineLayout pipelineLayout = VK_NULL_HANDLE;
};
