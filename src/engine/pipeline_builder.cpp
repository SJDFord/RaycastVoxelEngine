#include "pipeline_builder.hpp"

namespace engine {

PipelineBuilder::PipelineBuilder(vk::Device const &device): _device{device} {
  // TODO: Setters for all of these to allow overriding default pipeline behaviour
  _inputAssemblyInfo = vk::PipelineInputAssemblyStateCreateInfo()
    .setTopology(vk::PrimitiveTopology::eTriangleList)
    .setPrimitiveRestartEnable(false);

  _viewportInfo = vk::PipelineViewportStateCreateInfo()
    .setViewportCount(1)
    .setScissorCount(1);

  _rasterizationInfo = vk::PipelineRasterizationStateCreateInfo()
    .setDepthClampEnable(false)
    .setRasterizerDiscardEnable(false)
    .setPolygonMode(vk::PolygonMode::eFill)
    .setLineWidth(1.0f)
    .setCullMode(vk::CullModeFlagBits::eNone)
    .setFrontFace(vk::FrontFace::eClockwise)
    .setDepthBiasEnable(false)
    .setDepthBiasConstantFactor(0.0f)
    .setDepthBiasClamp(0.0f)
    .setDepthBiasSlopeFactor(0.0f);

  _multisampleInfo = vk::PipelineMultisampleStateCreateInfo()
    .setSampleShadingEnable(false)
    .setRasterizationSamples(vk::SampleCountFlagBits::e1)
    .setMinSampleShading(1.0f)
    .setAlphaToCoverageEnable(false)
    .setAlphaToOneEnable(false);

  _colorBlendAttachment = vk::PipelineColorBlendAttachmentState()
    .setColorWriteMask(
      vk::ColorComponentFlagBits::eR |
      vk::ColorComponentFlagBits::eG |
      vk::ColorComponentFlagBits::eB |
      vk::ColorComponentFlagBits::eA
    )
    .setBlendEnable(false)
    .setSrcColorBlendFactor(vk::BlendFactor::eOne)
    .setDstColorBlendFactor(vk::BlendFactor::eZero)
    .setColorBlendOp(vk::BlendOp::eAdd)
    .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
    .setDstAlphaBlendFactor(vk::BlendFactor::eZero)
    .setAlphaBlendOp(vk::BlendOp::eAdd);

  // TODO: _colorBlendAttachment shouldn't be it's own class member as this references it
  _colorBlendInfo = vk::PipelineColorBlendStateCreateInfo()
    .setLogicOpEnable(false)
    .setLogicOp(vk::LogicOp::eCopy)
    .setAttachments(_colorBlendAttachment)
    .setBlendConstants({0.0f, 0.0f, 0.0f, 0.0f});

  _depthStencilInfo = vk::PipelineDepthStencilStateCreateInfo()
    .setDepthTestEnable(false)
    .setDepthWriteEnable(false)
    .setDepthCompareOp(vk::CompareOp::eLess)
    .setDepthBoundsTestEnable(false)
    .setMinDepthBounds(0.0f)
    .setMaxDepthBounds(1.0f)
    .setStencilTestEnable(false)
    .setFront({})
    .setBack({});


  // TODO: _dynamicStateEnables shouldn't be their own class member as dynamic state info references it
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


vk::Pipeline PipelineBuilder::build() {
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

