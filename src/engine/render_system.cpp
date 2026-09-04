#include "render_system.hpp"

namespace engine {

RenderSystem::RenderSystem(
    vk::Device const &         device, 
    vk::PhysicalDevice const & physicalDevice,
    vk::DescriptorSetLayout globalSetLayout
) : _device{device}, _physicalDevice{physicalDevice}, _descriptorSetLayout{globalSetLayout} {
  createPipelineLayout(globalSetLayout);
  createPipeline();
}
RenderSystem::~RenderSystem() {

}


void RenderSystem::render(/*FrameInfo &frameInfo*/) {

}

void RenderSystem::createPipelineLayout(vk::DescriptorSetLayout globalSetLayout) {
  vk::PushConstantRange pushConstantRange{};
  pushConstantRange.setStageFlags(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);
  pushConstantRange.setOffset(0);
  pushConstantRange.setSize(sizeof(SimplePushConstantData));

  std::vector<vk::DescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

  vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.setSetLayouts(descriptorSetLayouts);
  pipelineLayoutInfo.setPushConstantRanges(pushConstantRange);
  _pipelineLayout = _device.createPipelineLayout(pipelineLayoutInfo);
}

void RenderSystem::createPipeline() {
    assert(_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

    engine::Texture textureData(_physicalDevice, _device);

    // TODO: Abstract into a builder pattern like InstanceBuilder for the vk::Instance
    engine::Buffer uniformBufferData(
        _physicalDevice,
        _device,
        sizeof(glm::mat4x4),
        vk::BufferUsageFlagBits::eUniformBuffer);

    std::string vertShaderGlsl = engine::readFileString("../shaders/vertexShaderText_PT_T.vert");
    std::string fragShaderGlsl = engine::readFileString("../shaders/fragmentShaderText_T_C.frag");
    vk::ShaderModule vertexShaderModule =
        engine::ShaderModuleBuilder(_device, vk::ShaderStageFlagBits::eVertex, vertShaderGlsl)
            .build();
    vk::ShaderModule fragmentShaderModule =
        engine::ShaderModuleBuilder(_device, vk::ShaderStageFlagBits::eFragment, fragShaderGlsl)
            .build();

    // TODO: Definitely builder pattern
    vk::DescriptorPool descriptorPool = vk::su::createDescriptorPool(
        _device,
        {{vk::DescriptorType::eUniformBuffer, 1}, {vk::DescriptorType::eCombinedImageSampler, 1}});
    vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo(descriptorPool, _descriptorSetLayout);
    vk::DescriptorSet descriptorSet =
        _device.allocateDescriptorSets(descriptorSetAllocateInfo).front();

    engine::updateDescriptorSets(
        _device,
        descriptorSet,
        {{vk::DescriptorType::eUniformBuffer, uniformBufferData.getBuffer(), VK_WHOLE_SIZE, {}}},
        textureData);

    vk::PipelineCache pipelineCache = _device.createPipelineCache(vk::PipelineCacheCreateInfo());

    // TODO: Definitely builder pattern
    /*
    _pipeline = vk::su::createGraphicsPipeline(
        device,
        pipelineCache,
        std::make_pair(vertexShaderModule, nullptr),
        std::make_pair(fragmentShaderModule, nullptr),
        sizeof(texturedCubeData[0]),
        {{vk::Format::eR32G32B32A32Sfloat, 0}, {vk::Format::eR32G32Sfloat, 16}},
        vk::FrontFace::eClockwise,
        true,
        pipelineLayout,
        VK_NULL_HANDLE,
        swapChainData.getFormat(),
        depthBufferData.getFormat(),
        enableDynamicRendering
    );
    */
    
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = {
        vk::PipelineShaderStageCreateInfo()
            .setStage(vk::ShaderStageFlagBits::eVertex)
            .setModule(vertexShaderModule)
            .setPName("main")
            //.setFlags()
            .setPNext(nullptr)
            .setPSpecializationInfo(nullptr),
        vk::PipelineShaderStageCreateInfo()
            .setStage(vk::ShaderStageFlagBits::eFragment)
            .setModule(fragmentShaderModule)
            .setPName("main")
            //.setFlags()
            .setPNext(nullptr)
            .setPSpecializationInfo(nullptr)
    };

    vk::PipelineShaderStageCreateInfo shaderStage1 = vk::PipelineShaderStageCreateInfo()
                                                        .setStage(vk::ShaderStageFlagBits::eVertex)
                                                        .setModule(vertexShaderModule)
                                                        .setPName("main")
                                                        //.setFlags()
                                                        .setPNext(nullptr)
                                                        .setPSpecializationInfo(nullptr);
    
    vk::PipelineShaderStageCreateInfo shaderStage2 = vk::PipelineShaderStageCreateInfo()
                                                        .setStage(vk::ShaderStageFlagBits::eFragment)
                                                        .setModule(fragmentShaderModule)
                                                        .setPName("main")
                                                        //.setFlags()
                                                        .setPNext(nullptr)
                                                        .setPSpecializationInfo(nullptr);
    
    

    //auto& bindingDescriptions = configInfo.bindingDescriptions;
    //auto& attributeDescriptions = configInfo.attributeDescriptions;
    /*
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
        .setSubpass(configInfo.subpass)
        .setBasePipelineIndex(-1)
        .setBasePipelineHandle(VK_NULL_HANDLE);
    
    graphicsPipeline = device->device().createGraphicsPipeline(VK_NULL_HANDLE, pipelineInfo);
    */
}

}