#include "pipeline_builder.hpp"


#include <print>

namespace engine {

PipelineBuilder::PipelineBuilder(vk::Device device, vk::PipelineLayout pipelineLayout): _device{device}, _pipelineLayout{pipelineLayout} {
  // TODO: Setters for all of these to allow overriding default pipeline behaviour
  _renderPass = VK_NULL_HANDLE;
  
  _inputAssemblyInfo = vk::PipelineInputAssemblyStateCreateInfo()
    .setTopology(vk::PrimitiveTopology::eTriangleList)
    .setPrimitiveRestartEnable(vk::False);

  _viewportInfo = vk::PipelineViewportStateCreateInfo()
    .setViewportCount(1)
    .setPViewports(nullptr)
    .setScissorCount(1)
    .setPScissors(nullptr);

  _rasterizationInfo = vk::PipelineRasterizationStateCreateInfo()
    .setDepthClampEnable(vk::False)
    .setRasterizerDiscardEnable(vk::False)
    .setPolygonMode(vk::PolygonMode::eFill)
    .setLineWidth(1.0f)
    .setCullMode(vk::CullModeFlagBits::eNone)
    .setFrontFace(vk::FrontFace::eClockwise)
    .setDepthBiasEnable(vk::False)
    .setDepthBiasConstantFactor(0.0f)
    .setDepthBiasClamp(0.0f)
    .setDepthBiasSlopeFactor(0.0f);

  _multisampleInfo = vk::PipelineMultisampleStateCreateInfo()
    .setSampleShadingEnable(vk::False)
    .setRasterizationSamples(vk::SampleCountFlagBits::e1)
    .setMinSampleShading(1.0f)
    .setPSampleMask(nullptr)
    .setAlphaToCoverageEnable(vk::False)
    .setAlphaToOneEnable(vk::False);

  setBlending(false);

  _depthStencilInfo = vk::PipelineDepthStencilStateCreateInfo()
    .setDepthTestEnable(vk::True)
    .setDepthWriteEnable(vk::True)
    .setDepthCompareOp(vk::CompareOp::eLess)
    .setDepthBoundsTestEnable(vk::False)
    .setMinDepthBounds(0.0f)
    .setMaxDepthBounds(1.0f)
    .setStencilTestEnable(vk::False)
    .setFront({})
    .setBack({});

  _dynamicStateEnables.push_back(vk::DynamicState::eViewport);
  _dynamicStateEnables.push_back(vk::DynamicState::eScissor);

  _dynamicStateInfo = vk::PipelineDynamicStateCreateInfo()
    .setDynamicStates(_dynamicStateEnables);
}

PipelineBuilder::~PipelineBuilder() {

}

PipelineBuilder& PipelineBuilder::addShaderModule(vk::ShaderModule shaderModule, vk::ShaderStageFlagBits shaderStage) {
  vk::PipelineShaderStageCreateInfo createInfo = vk::PipelineShaderStageCreateInfo()
            .setStage(shaderStage)
            .setModule(shaderModule)
            .setPName("main")
            .setPNext(nullptr)
            .setPSpecializationInfo(nullptr);
  _shaderStages.push_back(createInfo);
  return *this;
}


PipelineBuilder& PipelineBuilder::setRenderPass(vk::RenderPass renderPass) {
  _renderPass = renderPass;
  return *this;
}

PipelineBuilder& PipelineBuilder::addBindingDescription(vk::VertexInputBindingDescription bindingDescription) {
  _bindingDescriptions.push_back(bindingDescription);
  return *this;
}
PipelineBuilder& PipelineBuilder::addAttributeDescription(vk::VertexInputAttributeDescription attributeDescription) {
  _attributeDescriptions.push_back(attributeDescription);
  return *this;
}


PipelineBuilder& PipelineBuilder::setBindingDescriptions(std::vector<vk::VertexInputBindingDescription> bindingDescriptions) {
  _bindingDescriptions = bindingDescriptions;
  return *this;
}
PipelineBuilder& PipelineBuilder::setAttributeDescriptions(std::vector<vk::VertexInputAttributeDescription> attributeDescriptions) {
  _attributeDescriptions = attributeDescriptions;
  return *this;
}

PipelineBuilder& PipelineBuilder::setBlending(bool enableBlending) {
  if (enableBlending) {
    _colorBlendAttachment = vk::PipelineColorBlendAttachmentState()
      .setColorWriteMask(
        vk::ColorComponentFlagBits::eR |
        vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB |
        vk::ColorComponentFlagBits::eA
      )
      .setBlendEnable(vk::True)
      .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
      .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
      .setColorBlendOp(vk::BlendOp::eAdd)
      .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
      .setDstAlphaBlendFactor(vk::BlendFactor::eZero)
      .setAlphaBlendOp(vk::BlendOp::eAdd);
  } else {
    _colorBlendAttachment = vk::PipelineColorBlendAttachmentState()
      .setColorWriteMask(
        vk::ColorComponentFlagBits::eR |
        vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB |
        vk::ColorComponentFlagBits::eA
      )
      .setBlendEnable(vk::False)
      .setSrcColorBlendFactor(vk::BlendFactor::eOne)
      .setDstColorBlendFactor(vk::BlendFactor::eZero)
      .setColorBlendOp(vk::BlendOp::eAdd)
      .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
      .setDstAlphaBlendFactor(vk::BlendFactor::eZero)
      .setAlphaBlendOp(vk::BlendOp::eAdd);
  }
  return *this;
}

vk::Pipeline PipelineBuilder::build() {
  _colorBlendInfo = vk::PipelineColorBlendStateCreateInfo()
    .setLogicOpEnable(vk::False)
    .setLogicOp(vk::LogicOp::eCopy)
    .setAttachments(_colorBlendAttachment)
    .setBlendConstants({0.0f, 0.0f, 0.0f, 0.0f});
  std::println("Building pipeline...");
  vk::PipelineVertexInputStateCreateInfo vertexInputInfo = vk::PipelineVertexInputStateCreateInfo()
    .setVertexBindingDescriptions(_bindingDescriptions)
    .setVertexAttributeDescriptions(_attributeDescriptions);

  vk::GraphicsPipelineCreateInfo pipelineInfo = vk::GraphicsPipelineCreateInfo()
    .setStages(_shaderStages)
    .setPVertexInputState(&vertexInputInfo)
    .setPInputAssemblyState(&_inputAssemblyInfo)
    .setPViewportState(&_viewportInfo)
    .setPRasterizationState(&_rasterizationInfo)
    .setPMultisampleState(&_multisampleInfo)
    .setPColorBlendState(&_colorBlendInfo)
    .setPDepthStencilState(&_depthStencilInfo)
    .setPDynamicState(&_dynamicStateInfo)
    .setLayout(_pipelineLayout)
    .setRenderPass(_renderPass)
    .setSubpass(_subpass)
    .setBasePipelineIndex(-1)
    .setBasePipelineHandle(VK_NULL_HANDLE);

  vk::ResultValue<vk::Pipeline> pipelineResult = _device.createGraphicsPipeline(VK_NULL_HANDLE, pipelineInfo);

  if (pipelineResult.result != vk::Result::eSuccess) {
    throw std::runtime_error("failed to create graphics pipeline");
  }

  return pipelineResult.value;
}

}  // namespace engine

