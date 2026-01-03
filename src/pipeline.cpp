#include "pipeline.hpp"

#include "model.hpp"

// std
#include <cassert>
#include <fstream>
#include <iostream>
#include <stdexcept>

#ifndef ENGINE_DIR
#define ENGINE_DIR "../"
#endif

Pipeline::Pipeline(
    Device& device,
    const std::string& vertFilepath,
    const std::string& fragFilepath,
    const PipelineConfigInfo& configInfo)
    : device{device} {
  createGraphicsPipeline(vertFilepath, fragFilepath, configInfo);
}

Pipeline::~Pipeline() {
  //device.destroyShaderModule(vertShaderModule);
  //device.destroyShaderModule(fragShaderModule);
  //device.destroyPipeline(graphicsPipeline);
}

std::vector<char> Pipeline::readFile(const std::string& filepath) {
  std::string enginePath = ENGINE_DIR + filepath;
  std::ifstream file{enginePath, std::ios::ate | std::ios::binary};

  if (!file.is_open()) {
    throw std::runtime_error("failed to open file: " + enginePath);
  }

  size_t fileSize = static_cast<size_t>(file.tellg());
  std::vector<char> buffer(fileSize);

  file.seekg(0);
  file.read(buffer.data(), fileSize);

  file.close();
  return buffer;
}

void Pipeline::createGraphicsPipeline(
    const std::string& vertFilepath,
    const std::string& fragFilepath,
    const PipelineConfigInfo& configInfo) {
  /*
  *TODO:
  assert(
      configInfo.pipelineLayout != VK_NULL_HANDLE &&
      "Cannot create graphics pipeline: no pipelineLayout provided in configInfo");
  assert(
      configInfo.renderPass != VK_NULL_HANDLE &&
      "Cannot create graphics pipeline: no renderPass provided in configInfo");
  */

  auto vertCode = readFile(vertFilepath);
  auto fragCode = readFile(fragFilepath);

  createShaderModule(vertCode, vertShaderModule);
  createShaderModule(fragCode, fragShaderModule);

  std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = {
      vk::PipelineShaderStageCreateInfo()
          .setStage(vk::ShaderStageFlagBits::eVertex)
          .setModule(vertShaderModule)
          .setPName("main")
          //.setFlags()
          .setPNext(nullptr)
          .setPSpecializationInfo(nullptr),
      vk::PipelineShaderStageCreateInfo()
          .setStage(vk::ShaderStageFlagBits::eFragment)
          .setModule(fragShaderModule)
          .setPName("main")
          //.setFlags()
          .setPNext(nullptr)
          .setPSpecializationInfo(nullptr)
  };

  vk::PipelineShaderStageCreateInfo shaderStage1 = vk::PipelineShaderStageCreateInfo()
                                                       .setStage(vk::ShaderStageFlagBits::eVertex)
                                                       .setModule(vertShaderModule)
                                                       .setPName("main")
                                                       //.setFlags()
                                                       .setPNext(nullptr)
                                                       .setPSpecializationInfo(nullptr);
  
  vk::PipelineShaderStageCreateInfo shaderStage2 = vk::PipelineShaderStageCreateInfo()
                                                       .setStage(vk::ShaderStageFlagBits::eFragment)
                                                       .setModule(fragShaderModule)
                                                       .setPName("main")
                                                       //.setFlags()
                                                       .setPNext(nullptr)
                                                       .setPSpecializationInfo(nullptr);
   
  

  auto& bindingDescriptions = configInfo.bindingDescriptions;
  auto& attributeDescriptions = configInfo.attributeDescriptions;
  vk::PipelineVertexInputStateCreateInfo vertexInputInfo = vk::PipelineVertexInputStateCreateInfo()
      .setVertexAttributeDescriptionCount(static_cast<uint32_t>(attributeDescriptions.size()))
      .setVertexBindingDescriptionCount(static_cast<uint32_t>(bindingDescriptions.size()))
      .setVertexAttributeDescriptions(attributeDescriptions)
      .setVertexBindingDescriptions(bindingDescriptions);
  
  vk::GraphicsPipelineCreateInfo pipelineInfo = vk::GraphicsPipelineCreateInfo()
      .setStageCount(2)
      .setStages(shaderStages)
      .setPVertexInputState(&vertexInputInfo)
      .setPInputAssemblyState(&configInfo.inputAssemblyInfo)
      .setPViewportState(&configInfo.viewportInfo)
      .setPRasterizationState(&configInfo.rasterizationInfo)
      .setPMultisampleState(&configInfo.multisampleInfo)
      .setPColorBlendState(&configInfo.colorBlendInfo)
      .setPDepthStencilState(&configInfo.depthStencilInfo)
      .setPDynamicState(&configInfo.dynamicStateInfo)
      .setLayout(configInfo.pipelineLayout)
      .setRenderPass(configInfo.renderPass)
      .setSubpass(configInfo.subpass)
      .setBasePipelineIndex(-1)
      .setBasePipelineHandle(VK_NULL_HANDLE);
  
  graphicsPipeline = device.device().createGraphicsPipeline(VK_NULL_HANDLE, pipelineInfo);
}

void Pipeline::createShaderModule(const std::vector<char>& code, vk::raii::ShaderModule& shaderModule) {
  vk::ShaderModuleCreateInfo createInfo = vk::ShaderModuleCreateInfo()
      .setCodeSize(code.size())
      .setPCode(reinterpret_cast<const uint32_t*>(code.data()));
  
  shaderModule = device.device().createShaderModule(createInfo);
}

void Pipeline::bind(const vk::raii::CommandBuffer& commandBuffer) {
  commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);
}

void Pipeline::defaultPipelineConfigInfo(PipelineConfigInfo& configInfo) {
  configInfo.inputAssemblyInfo.setTopology(vk::PrimitiveTopology::eTriangleList);
  configInfo.inputAssemblyInfo.setPrimitiveRestartEnable(false);

  configInfo.viewportInfo.setViewportCount(1);
  configInfo.viewportInfo.setPViewports(nullptr);
  configInfo.viewportInfo.setScissorCount(1);
  configInfo.viewportInfo.setPScissors(nullptr);

  configInfo.rasterizationInfo.setDepthClampEnable(false);
  configInfo.rasterizationInfo.setRasterizerDiscardEnable(false);
  configInfo.rasterizationInfo.setPolygonMode(vk::PolygonMode::eFill);
  configInfo.rasterizationInfo.setLineWidth(1.0f);
  configInfo.rasterizationInfo.setCullMode(vk::CullModeFlagBits::eNone);
  configInfo.rasterizationInfo.setFrontFace(vk::FrontFace::eClockwise);
  configInfo.rasterizationInfo.setDepthBiasEnable(false);
  configInfo.rasterizationInfo.setDepthBiasConstantFactor(0.0f);
  configInfo.rasterizationInfo.setDepthBiasClamp(0.0f);
  configInfo.rasterizationInfo.setDepthBiasSlopeFactor(0.0f);

  configInfo.multisampleInfo.setSampleShadingEnable(false);
  configInfo.multisampleInfo.setRasterizationSamples(vk::SampleCountFlagBits::e1);
  configInfo.multisampleInfo.setMinSampleShading(1.0f);
  configInfo.multisampleInfo.setPSampleMask(nullptr);
  configInfo.multisampleInfo.setAlphaToCoverageEnable(false);
  configInfo.multisampleInfo.setAlphaToOneEnable(false);
  
  configInfo.colorBlendAttachment.setColorWriteMask(
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);

  configInfo.colorBlendAttachment.setBlendEnable(false);
  configInfo.colorBlendAttachment.setSrcColorBlendFactor(vk::BlendFactor::eOne);
  configInfo.colorBlendAttachment.setDstColorBlendFactor(vk::BlendFactor::eZero);
  configInfo.colorBlendAttachment.setColorBlendOp(vk::BlendOp::eAdd);
  configInfo.colorBlendAttachment.setSrcAlphaBlendFactor(vk::BlendFactor::eOne);
  configInfo.colorBlendAttachment.setDstAlphaBlendFactor(vk::BlendFactor::eZero);
  configInfo.colorBlendAttachment.setAlphaBlendOp(vk::BlendOp::eAdd);

  configInfo.colorBlendInfo.setLogicOpEnable(false);
  configInfo.colorBlendInfo.setLogicOp(vk::LogicOp::eCopy);
  configInfo.colorBlendInfo.setAttachmentCount(1);
  configInfo.colorBlendInfo.setAttachments(configInfo.colorBlendAttachment);
  std::array<float, 4Ui64> blendConstants = {0.0f, 0.0f, 0.0f, 0.0f};
  configInfo.colorBlendInfo.setBlendConstants(blendConstants);

  configInfo.depthStencilInfo.setDepthTestEnable(true);
  configInfo.depthStencilInfo.setDepthWriteEnable(true);
  configInfo.depthStencilInfo.setDepthCompareOp(vk::CompareOp::eLess);
  configInfo.depthStencilInfo.setDepthBoundsTestEnable(false);
  configInfo.depthStencilInfo.setMinDepthBounds(0.0f);
  configInfo.depthStencilInfo.setMaxDepthBounds(1.0f);
  configInfo.depthStencilInfo.setStencilTestEnable(false);
  /*
  configInfo.depthStencilInfo.front = {};  // Optional
  configInfo.depthStencilInfo.back = {};   // Optional
  */

  configInfo.dynamicStateEnables = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
  configInfo.dynamicStateInfo.setDynamicStates(configInfo.dynamicStateEnables);
  configInfo.dynamicStateInfo.setDynamicStateCount(
      static_cast<uint32_t>(configInfo.dynamicStateEnables.size()));
  
  //configInfo.dynamicStateInfo.flags = 0;

  configInfo.bindingDescriptions = Vertex::getBindingDescriptionsCpp();
  configInfo.attributeDescriptions = Vertex::getAttributeDescriptionsCpp();
}

void Pipeline::enableAlphaBlending(PipelineConfigInfo& configInfo) {
  configInfo.colorBlendAttachment.setBlendEnable(true);
  configInfo.colorBlendAttachment.setColorWriteMask(
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
      
  configInfo.colorBlendAttachment.setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha);
  configInfo.colorBlendAttachment.setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrc1Alpha);
  configInfo.colorBlendAttachment.setColorBlendOp(vk::BlendOp::eAdd);
  configInfo.colorBlendAttachment.setSrcAlphaBlendFactor(vk::BlendFactor::eOne);
  configInfo.colorBlendAttachment.setDstAlphaBlendFactor(vk::BlendFactor::eZero);
  configInfo.colorBlendAttachment.setAlphaBlendOp(vk::BlendOp::eAdd);
}
