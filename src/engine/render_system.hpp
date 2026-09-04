#pragma once

#include <memory>

#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_raii.hpp"
#include "utils.hpp"
#include "../utils/utils.hpp"

#include "shader_module_builder.hpp"
#include "texture.hpp"
#include "buffer.hpp"
#include "descriptor_set_utils.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace engine {
struct SimplePushConstantData {
  glm::mat4 modelMatrix{1.f};
  glm::mat4 normalMatrix{1.f};
};

class RenderSystem {
public:
    RenderSystem(
        vk::Device const &         device, 
        vk::PhysicalDevice const & physicalDevice,
        vk::DescriptorSetLayout globalSetLayout
    );
    ~RenderSystem();

    RenderSystem(const RenderSystem&) = delete;
    RenderSystem& operator=(const RenderSystem&) = delete;


    void render(/*FrameInfo &frameInfo*/);
private:
  void createPipelineLayout(vk::DescriptorSetLayout globalSetLayout);
  void createPipeline();

  const vk::Device& _device;
  const vk::PhysicalDevice& _physicalDevice;
  const vk::DescriptorSetLayout& _descriptorSetLayout; 
  
  vk::Pipeline _pipeline;
  vk::PipelineLayout _pipelineLayout;
};

}
