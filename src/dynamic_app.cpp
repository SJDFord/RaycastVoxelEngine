#include "dynamic_app.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <chrono>
#include <stdexcept>
#include <thread>
#include <print>

#include "./engine/buffer.hpp"
#include "./engine/descriptor_set_layout_builder.hpp"
#include "./engine/descriptor_set_utils.hpp"
#include "./engine/device_builder.hpp"
#include "./engine/image_from_file.hpp"
#include "./engine/image_generators.hpp"
#include "./engine/instance_builder.hpp"
#include "./engine/physical_device_strategy.hpp"
#include "./engine/ranked_physical_device_strategy.hpp"
#include "./engine/shader_module_builder.hpp"
#include "./engine/swap_chain.hpp"
#include "./engine/texture.hpp"
#include "./engine/utils.hpp"
#include "./engine/pipeline_builder.hpp"
#include "./engine/vertex.hpp"
#include "./engine/render_system.hpp"

#include "glslang/Public/ShaderLang.h"

DynamicApp::DynamicApp() : _window{APP_NAME, WIDTH, HEIGHT} {
  init();
}

DynamicApp::~DynamicApp() {}

void DynamicApp::run() {

    vk::DescriptorPool descriptorPool = engine::DescriptorPoolBuilder(_device)
        .setMaxSets(MAX_FRAMES_IN_FLIGHT)
        .addPoolSize(vk::DescriptorType::eUniformBuffer, MAX_FRAMES_IN_FLIGHT)
        .addPoolSize(vk::DescriptorType::eCombinedImageSampler, MAX_FRAMES_IN_FLIGHT)
        .build();

    std::vector<std::unique_ptr<engine::Buffer>> uboBuffers(MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < uboBuffers.size(); i++) {
        vk::DeviceSize size = vk::DeviceSize(sizeof(engine::GlobalUbo));
        uboBuffers[i] = std::make_unique<engine::Buffer>(
            _physicalDevice,
            _device,
            size,
            vk::BufferUsageFlagBits::eUniformBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible);
        uboBuffers[i]->map();
    }

    engine::Renderer renderer(
        _window, 
        _device, 
        _physicalDevice, 
        vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc,
        _graphicsQueueIndex,
        _presentQueueIndex,
        MAX_FRAMES_IN_FLIGHT
    );

    std::println("Renderer created");
    /*
    auto oneTimeCommandBuffer = renderer.beginOneTimeCommandBuffer();

    // TODO: Align engine::Image with image.cpp (or allow creating an image from file path in another way)
    engine::ImageFromFile image = engine::ImageFromFile(
        _physicalDevice,
        _device,
        oneTimeCommandBuffer,
        "../textures/jungle-brick-with-moss.png"
    );
    
    renderer.endOneTimeCommandBuffer(oneTimeCommandBuffer);
    */

    /*
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = image->getImageView();
    imageInfo.sampler = image->getSampler();

    std::vector<VkDescriptorSet> globalDescriptorSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < globalDescriptorSets.size(); i++) {
        auto bufferInfo = uboBuffers[i]->descriptorInfo();
        LveDescriptorWriter(*globalSetLayout, *globalPool)
            .writeBuffer(0, &bufferInfo)
            .writeImage(1, &imageInfo)
            .build(globalDescriptorSets[i]);
    }
    */


    auto currentTime = std::chrono::high_resolution_clock::now();
    while (!_window.shouldClose()) {
        _window.pollEvents();

        if (_window.isKeyPressed(engine::KeyboardKey::ESCAPE)) {
            _window.close();
        }

        
        auto newTime = std::chrono::high_resolution_clock::now();
        float frameTime =
            std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
        currentTime = newTime;

        std::println("aspect frame...");

        //float aspect = _renderer->getAspectRatio();

        auto extent = _window.getExtent();
        float aspect = static_cast<float>(extent.width) / static_cast<float>(extent.height);


        std::println("beginning frame...");
        if (auto commandBuffer = _renderer->beginFrame(/*hasFrame*/)) {

            std::println("frame...");
            int frameIndex = _renderer->getFrameIndex();
            /*
            FrameInfo frameInfo{
                frameIndex,
                frameTime,
                commandBuffer,
                camera,
                globalDescriptorSets[frameIndex],
                gameObjects};

            // update
            GlobalUbo ubo{};
            ubo.projection = camera.getProjection();
            ubo.view = camera.getView();
            ubo.inverseView = camera.getInverseView();
            pointLightSystem.update(frameInfo, ubo);
            uboBuffers[frameIndex]->writeToBuffer(&ubo);
            uboBuffers[frameIndex]->flush();

            // render
            //lveRenderer.beginSwapChainRenderPass(commandBuffer);

            // order here matters
            simpleRenderSystem.renderGameObjects(frameInfo);
            pointLightSystem.render(frameInfo);
            */
            //renderer.endSwapChainRenderPass(commandBuffer);
            _renderer->endFrame();
        }
        
  }

  _device.waitIdle();
}

void DynamicApp::init() {
    char const* EngineName = "TEST";
    std::println("Window created...");
    std::vector<std::string> extensions = _window.getRequiredExtensions();

    for (int i = 0; i < extensions.size(); i++) {
        std::cout << "Extension required: " << extensions[i] << std::endl; 
    }

    vk::Instance instance = engine::InstanceBuilder(APP_NAME, EngineName, VK_API_VERSION_1_3)
                                .setExtensions(extensions)
                                .build();

    std::println("Instance created...");
    _window.createSurface(instance);

#if !defined(NDEBUG)
    // TODO: Do this in the InstanceBuilder instead
    std::println("Debug messenger");
    //vk::DebugUtilsMessengerEXT debugUtilsMessenger = instance.createDebugUtilsMessengerEXT(
    //    vk::su::makeDebugUtilsMessengerCreateInfoEXT() 
    //);
#endif

    const std::vector<vk::PhysicalDevice>& physicalDevices = instance.enumeratePhysicalDevices();
    engine::RankedPhysicalDeviceStrategy physicalDeviceStrategy{};
    _physicalDevice = physicalDeviceStrategy.pickPhysicalDevice(physicalDevices);

    std::pair<uint32_t, uint32_t> queueFamilyIndices =
        engine::findGraphicsAndPresentQueueFamilyIndex(_physicalDevice, _window.getSurface());
    _graphicsQueueIndex = queueFamilyIndices.first;
    _presentQueueIndex = queueFamilyIndices.second;
    std::println("Physical device created...");

    _device = engine::DeviceBuilder(_physicalDevice, _graphicsQueueIndex)
        .setExtensions({
            VK_KHR_SWAPCHAIN_EXTENSION_NAME, 
            VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME
        })
        .setPNext(new vk::PhysicalDeviceDynamicRenderingFeatures(VK_TRUE))
        .build();

    std::println("Device created...");

    std::string vertShaderGlsl = engine::readFileString("../shaders/vertexShaderText_PT_T.vert");
    std::string fragShaderGlsl = engine::readFileString("../shaders/fragmentShaderText_T_C.frag");
    vk::ShaderModule vertexShaderModule =
        engine::ShaderModuleBuilder(_device, vk::ShaderStageFlagBits::eVertex, vertShaderGlsl)
            .build();
    vk::ShaderModule fragmentShaderModule =
        engine::ShaderModuleBuilder(_device, vk::ShaderStageFlagBits::eFragment, fragShaderGlsl)
            .build();
    
    std::println("Shaders created...");

    vk::DescriptorSetLayout descriptorSetLayout =
        engine::DescriptorSetLayoutBuilder(_device)
            .addBinding(vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex)
            .addBinding(
                vk::DescriptorType::eCombinedImageSampler,
                1,
                vk::ShaderStageFlagBits::eFragment)
            .build();

    vk::PushConstantRange pushConstantRange{};
    pushConstantRange.setStageFlags(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);
    pushConstantRange.setOffset(0);
    pushConstantRange.setSize(sizeof(engine::SimplePushConstantData));

    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts{descriptorSetLayout};

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.setSetLayouts(descriptorSetLayouts);
    pipelineLayoutInfo.setPushConstantRanges(pushConstantRange);
    auto pipelineLayout = _device.createPipelineLayout(pipelineLayoutInfo);

    std::println("Pipeline Layout created...");

    vk::Pipeline pipeline = engine::PipelineBuilder(_device, pipelineLayout)
                .addShaderModule(vertexShaderModule, vk::ShaderStageFlagBits::eVertex)
                .addShaderModule(fragmentShaderModule, vk::ShaderStageFlagBits::eFragment)
                .setBindingDescriptions(engine::Vertex::getBindingDescriptions())
                .setAttributeDescriptions(engine::Vertex::getAttributeDescriptions())
                .build();

    std::println("Pipeline created...");
}


void DynamicApp::load() {

}