#pragma once

#include "device.hpp"

// std
#include <string>
#include <vector>

struct PipelineConfigInfo {
  PipelineConfigInfo(
      const vk::raii::PipelineLayout& pipelineLayout, const vk::raii::RenderPass& renderPass)
      : pipelineLayout{pipelineLayout}, renderPass{renderPass} {
  };
  PipelineConfigInfo(const PipelineConfigInfo&) = delete;
  PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

  std::vector<vk::VertexInputBindingDescription> bindingDescriptions{};
  std::vector<vk::VertexInputAttributeDescription> attributeDescriptions{};
  vk::PipelineViewportStateCreateInfo viewportInfo;
  vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
  vk::PipelineRasterizationStateCreateInfo rasterizationInfo;
  vk::PipelineMultisampleStateCreateInfo multisampleInfo;
  vk::PipelineColorBlendAttachmentState colorBlendAttachment;
  vk::PipelineColorBlendStateCreateInfo colorBlendInfo;
  vk::PipelineDepthStencilStateCreateInfo depthStencilInfo;
  std::vector<vk::DynamicState> dynamicStateEnables;
  vk::PipelineDynamicStateCreateInfo dynamicStateInfo;
  const vk::raii::PipelineLayout& pipelineLayout;
  const vk::raii::RenderPass& renderPass;
  uint32_t subpass = 0;
};

class Pipeline {
 public:
  Pipeline(
      std::shared_ptr<Device> device,
      const std::string& vertFilepath,
      const std::string& fragFilepath,
      const PipelineConfigInfo& configInfo);
  ~Pipeline();

  Pipeline(const Pipeline&) = delete;
  Pipeline& operator=(const Pipeline&) = delete;

  void bind(const vk::raii::CommandBuffer& commandBuffer);

  static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);
  static void enableAlphaBlending(PipelineConfigInfo& configInfo);

 private:
  static std::vector<char> readFile(const std::string& filepath);

  void createGraphicsPipeline(
      const std::string& vertFilepath,
      const std::string& fragFilepath,
      const PipelineConfigInfo& configInfo);

  void createShaderModule(const std::vector<char>& code, vk::raii::ShaderModule& shaderModule);

  std::shared_ptr<Device> device;
  vk::raii::Pipeline graphicsPipeline = VK_NULL_HANDLE;
  vk::raii::ShaderModule vertShaderModule = VK_NULL_HANDLE;
  vk::raii::ShaderModule fragShaderModule = VK_NULL_HANDLE;
};
