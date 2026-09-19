#pragma once

#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <iostream>
#include <string>
#include <vector>

namespace engine {

class PipelineBuilder {
 public:
  PipelineBuilder(vk::Device const &device);
  ~PipelineBuilder();

  PipelineBuilder(const PipelineBuilder&) = delete;
  PipelineBuilder& operator=(const PipelineBuilder&) = delete;


  PipelineBuilder& addShaderModule(vk::ShaderModule shaderModule, vk::ShaderStageFlagBits shaderStage);

  PipelineBuilder& addBindingDescription(vk::VertexInputBindingDescription bindingDescription);
  PipelineBuilder& addAttributeDescription(vk::VertexInputAttributeDescription attributeDescription);

  PipelineBuilder& setBindingDescriptions(std::vector<vk::VertexInputBindingDescription> bindingDescriptions);
  PipelineBuilder& setAttributeDescriptions(std::vector<vk::VertexInputAttributeDescription> attributeDescriptions);

  vk::Pipeline build();

 private:
  const vk::Device& _device;

  std::vector<vk::PipelineShaderStageCreateInfo> _shaderStages;
  std::vector<vk::VertexInputBindingDescription> _bindingDescriptions{};
  std::vector<vk::VertexInputAttributeDescription> _attributeDescriptions{};
  
  vk::PipelineInputAssemblyStateCreateInfo _inputAssemblyInfo;
  vk::PipelineViewportStateCreateInfo _viewportInfo;
  vk::PipelineRasterizationStateCreateInfo _rasterizationInfo;
  vk::PipelineMultisampleStateCreateInfo _multisampleInfo;
  vk::PipelineColorBlendAttachmentState _colorBlendAttachment;
  vk::PipelineColorBlendStateCreateInfo _colorBlendInfo;
  vk::PipelineDepthStencilStateCreateInfo _depthStencilInfo;
  std::vector<vk::DynamicState> _dynamicStateEnables;
  vk::PipelineDynamicStateCreateInfo _dynamicStateInfo;
  vk::PipelineLayout _pipelineLayout;
  uint32_t _subpass = 0;

  std::vector<std::string> _extensions;
};

}  // namespace engine
