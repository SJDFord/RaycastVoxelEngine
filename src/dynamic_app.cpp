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
#include "./engine/image.hpp"
#include "./engine/image_generators.hpp"
#include "./engine/instance_builder.hpp"
#include "./engine/physical_device_strategy.hpp"
#include "./engine/ranked_physical_device_strategy.hpp"
#include "./engine/shader_module_builder.hpp"
#include "./engine/swap_chain.hpp"
#include "./engine/texture.hpp"
#include "./engine/utils.hpp"
#include "./engine/window.hpp"
#include "./engine/renderer.hpp"
#include "./utils/geometries.hpp"
#include "./utils/math.hpp"
#include "./utils/shaders.hpp"
#include "./utils/utils.hpp"
#include "device.hpp"
#include "glslang/Public/ShaderLang.h"
#include "window.hpp"

DynamicApp::DynamicApp() {
 
}

DynamicApp::~DynamicApp() {}

void DynamicApp::run() {
    char const* AppName = "DynamicApp";
    char const* EngineName = "TEST";

    engine::Window window(AppName, 500, 500);
    std::println("Window created...");
    std::vector<std::string> extensions = window.getRequiredExtensions();

    for (int i = 0; i < extensions.size(); i++) {
        std::cout << "Extension required: " << extensions[i] << std::endl; 
    }

    vk::Instance instance = engine::InstanceBuilder(AppName, EngineName, VK_API_VERSION_1_3)
                                .setExtensions(extensions)
                                .build();

    std::println("Instance created...");
    window.createSurface(instance);

#if !defined(NDEBUG)
    // TODO: Do this in the InstanceBuilder instead
    std::println("Debug messenger");
    //vk::DebugUtilsMessengerEXT debugUtilsMessenger = instance.createDebugUtilsMessengerEXT(
    //    vk::su::makeDebugUtilsMessengerCreateInfoEXT() 
    //);
#endif

    const std::vector<vk::PhysicalDevice>& physicalDevices = instance.enumeratePhysicalDevices();
    engine::RankedPhysicalDeviceStrategy physicalDeviceStrategy{};
    vk::PhysicalDevice physicalDevice = physicalDeviceStrategy.pickPhysicalDevice(physicalDevices);

    std::pair<uint32_t, uint32_t> queueFamilyIndices =
        engine::findGraphicsAndPresentQueueFamilyIndex(physicalDevice, window.getSurface());
    uint32_t graphicsQueueIndex = queueFamilyIndices.first;
    uint32_t presentQueueIndex = queueFamilyIndices.second;
    std::println("Physical device created...");

    vk::Device device = engine::DeviceBuilder(physicalDevice, graphicsQueueIndex)
        .setExtensions({
            VK_KHR_SWAPCHAIN_EXTENSION_NAME, 
            VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME
        })
        .setPNext(new vk::PhysicalDeviceDynamicRenderingFeatures(VK_TRUE))
        .build();

    std::println("Device created...");

    engine::Renderer renderer(
        window, 
        device, 
        physicalDevice, 
        vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc,
        graphicsQueueIndex,
        presentQueueIndex,
        MAX_FRAMES_IN_FLIGHT
    );

    std::println("Renderer created...");

    /* TODO: Render pipeline/render system logic
    vk::DescriptorSetLayout descriptorSetLayout =
        engine::DescriptorSetLayoutBuilder(device)
            .addBinding(vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex)
            .addBinding(
                vk::DescriptorType::eCombinedImageSampler,
                1,
                vk::ShaderStageFlagBits::eFragment)
            .build();

    vk::PipelineLayout pipelineLayout = device.createPipelineLayout(
        vk::PipelineLayoutCreateInfo(vk::PipelineLayoutCreateFlags(), descriptorSetLayout));

    std::string vertShaderGlsl = engine::readFileString("../shaders/vertexShaderText_PT_T.vert");
    std::string fragShaderGlsl = engine::readFileString("../shaders/fragmentShaderText_T_C.frag");
    vk::ShaderModule vertexShaderModule =
        engine::ShaderModuleBuilder(device, vk::ShaderStageFlagBits::eVertex, vertShaderGlsl)
            .build();
    vk::ShaderModule fragmentShaderModule =
        engine::ShaderModuleBuilder(device, vk::ShaderStageFlagBits::eFragment, fragShaderGlsl)
            .build();

    // TODO: Definitely builder pattern
    vk::DescriptorPool descriptorPool = vk::su::createDescriptorPool(
        device,
        {{vk::DescriptorType::eUniformBuffer, 1}, {vk::DescriptorType::eCombinedImageSampler, 1}});
    vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo(descriptorPool, descriptorSetLayout);
    vk::DescriptorSet descriptorSet =
        device.allocateDescriptorSets(descriptorSetAllocateInfo).front();

    engine::updateDescriptorSets(
        device,
        descriptorSet,
        {{vk::DescriptorType::eUniformBuffer, uniformBufferData.getBuffer(), VK_WHOLE_SIZE, {}}},
        textureData);

    vk::PipelineCache pipelineCache = device.createPipelineCache(vk::PipelineCacheCreateInfo());

    // TODO: Definitely builder pattern
    vk::Pipeline graphicsPipeline = vk::su::createGraphicsPipeline(
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

    std::println("Render pipeline created");

    auto currentTime = std::chrono::high_resolution_clock::now();
    while (!window.shouldClose()) {
        window.pollEvents();

        if (window.isKeyPressed(engine::KeyboardKey::ESCAPE)) {
            window.close();
        }

        
        auto newTime = std::chrono::high_resolution_clock::now();
        float frameTime =
            std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
        currentTime = newTime;

        float aspect = renderer.getAspectRatio();

        if (auto commandBuffer = renderer.beginFrame(/*hasFrame*/)) {

            std::println("frame...");
            int frameIndex = renderer.getFrameIndex();
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
            renderer.endFrame();
        }
        
  }

  device.waitIdle();
}


void DynamicApp::load() {

}