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
#include "./engine/image.hpp"
#include "./engine/image_generators.hpp"
#include "./engine/shader_module_builder.hpp"
#include "./engine/swap_chain.hpp"
#include "./engine/utils.hpp"
#include "./engine/pipeline_builder.hpp"
#include "./engine/vertex.hpp"
#include "./engine/descriptors.hpp"
#include "./engine/fps_movement_controller.hpp"
#include "./engine/camera.hpp"
#include "./engine/one_time_command_submitter.hpp"


#include "glslang/Public/ShaderLang.h"

DynamicApp::DynamicApp() {
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
        uboBuffers[i] = std::make_unique<engine::Buffer>(
            _device,
            _physicalDevice,
            sizeof(engine::GlobalUbo),
            1,
            vk::BufferUsageFlagBits::eUniformBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible);
        uboBuffers[i]->map();
    }

    auto commandPool = _device.createCommandPool({vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _graphicsQueueIndex});

    engine::Renderer renderer(
        _window, 
        _surface,
        _device, 
        _physicalDevice, 
        commandPool,
        _graphicsQueueIndex,
        _presentQueueIndex,
        MAX_FRAMES_IN_FLIGHT
    );

    std::println("Renderer created");

    std::string vertShaderGlsl = engine::readFileString("../shaders/simple_shader.vert");
    std::string fragShaderGlsl = engine::readFileString("../shaders/simple_shader.frag");
    vk::ShaderModule vertexShaderModule =
        engine::ShaderModuleBuilder(_device, vk::ShaderStageFlagBits::eVertex, vertShaderGlsl)
            .build();
    vk::ShaderModule fragmentShaderModule =
        engine::ShaderModuleBuilder(_device, vk::ShaderStageFlagBits::eFragment, fragShaderGlsl)
            .build();
    
    std::println("Shaders created...");
    vk::RenderPass renderPass = renderer.getSwapChainRenderPass();
    assert(renderPass != VK_NULL_HANDLE && "Render Pass is not initialised");

    _pipeline = engine::PipelineBuilder(_device, _pipelineLayout)
                .addShaderModule(vertexShaderModule, vk::ShaderStageFlagBits::eVertex)
                .addShaderModule(fragmentShaderModule, vk::ShaderStageFlagBits::eFragment)
                .setRenderPass(renderPass)
                .setBindingDescriptions(engine::Vertex::getBindingDescriptions())
                .setAttributeDescriptions(engine::Vertex::getAttributeDescriptions())
                .build();

    std::println("Pipeline created...");
    
    auto commandSubmitter = engine::OneTimeCommandSubmitter(_device, commandPool, _graphicsQueueIndex);

    engine::Image image = engine::Image(
        _device,
        _physicalDevice,
        commandSubmitter,
        std::string("../textures/jungle-brick-with-moss.png")
    );
        
    auto testGameObject = engine::GameObject::createGameObject();
    glm::vec3 position = {1.0f, 1.0f, 1.0f};
    auto testMesh = engine::createCubeMesh(position, {0.0f, 0.5f, 0.5f}, true, true, true, true, true, true);
    std::shared_ptr<engine::Model> testModel = std::make_shared<engine::Model>(_device, _physicalDevice, commandSubmitter, testMesh);
    testGameObject.model = testModel;
    testGameObject.transform.translation = position;  // chunk.Position * (float)chunk.Size;
    testGameObject.transform.scale = {0.2f, 0.2f, 0.2f};
    _gameObjects.emplace(testGameObject.getId(), std::move(testGameObject));

    auto testGameObject2 = engine::GameObject::createGameObject();
    glm::vec3 position2 = {2.0f, 1.0f, 1.0f};
    auto testMesh2 = engine::createCubeMesh(position2, {1.0f, 0.65f, 0.0f}, true, true, true, true, true, true);
    std::shared_ptr<engine::Model> testModel2 = std::make_shared<engine::Model>(_device, _physicalDevice, commandSubmitter, testMesh2);
    testGameObject2.model = testModel2;
    testGameObject2.transform.translation = position2;  // chunk.Position * (float)chunk.Size;
    testGameObject2.transform.scale = {2.0f, 2.0f, 2.0f};
    _gameObjects.emplace(testGameObject2.getId(), std::move(testGameObject2));

    std::println("ImageFromFile created");

    
    vk::DescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    imageInfo.imageView = image.getImageView();
    imageInfo.sampler = image.getSampler();

    std::vector<vk::DescriptorSet> globalDescriptorSets(MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < globalDescriptorSets.size(); i++) {
        auto bufferInfo = uboBuffers[i]->descriptorInfo();
        //engine::
        globalDescriptorSets[i] = engine::DescriptorWriter(_device, descriptorPool, _descriptorSetLayout, _descriptorSetLayoutBindings)
            .writeBuffer(0, &bufferInfo)
            .writeImage(1, &imageInfo)
            .build();
    }

    engine::Camera camera{};
    auto viewerObject = engine::GameObject::createGameObject();
    viewerObject.transform.translation.z = -2.5f;
    engine::FpsMovementController cameraController{_window};

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


        cameraController.updateView(_window, frameTime, viewerObject);
        camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

        //float aspect = _renderer->getAspectRatio();

        auto extent = _window.getExtent();
        float aspect = static_cast<float>(extent.width) / static_cast<float>(extent.height);
        camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);



        vk::CommandBuffer commandBuffer = renderer.beginFrame(/*hasFrame*/);

        if (commandBuffer != VK_NULL_HANDLE) {
            int frameIndex = renderer.getFrameIndex();

            engine::GlobalUbo ubo{};
            ubo.projection = camera.getProjection();
            ubo.view = camera.getView();
            ubo.inverseView = camera.getInverseView();
            uboBuffers[frameIndex]->writeToBuffer(&ubo);
            uboBuffers[frameIndex]->flush();


            // render
            renderer.beginSwapChainRenderPass(commandBuffer);
            commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _pipeline);
            commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _pipelineLayout, 0, globalDescriptorSets, {});
            
            for (auto& kv : _gameObjects) {
                auto& obj = kv.second;
                if (obj.model == nullptr) continue;
                engine::SimplePushConstantData push{};
                push.modelMatrix = obj.transform.mat4();
                push.normalMatrix = obj.transform.normalMatrix();

                commandBuffer.pushConstants(
                    _pipelineLayout, 
                    vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
                    0,
                sizeof(engine::SimplePushConstantData),
        &push);

                obj.model->bind(commandBuffer);
                obj.model->draw(commandBuffer);
            }

            renderer.endSwapChainRenderPass(commandBuffer);
            renderer.endFrame();
        }
        
  }

  _device.waitIdle();
}

void DynamicApp::init() {
    char const* EngineName = "TEST";
    std::println("Window created...");

#if !defined(NDEBUG)
    // TODO: Do this in the InstanceBuilder instead
    std::println("Debug messenger");
    //vk::DebugUtilsMessengerEXT debugUtilsMessenger = instance.createDebugUtilsMessengerEXT(
    //    vk::su::makeDebugUtilsMessengerCreateInfoEXT() 
    //);
#endif

    const std::vector<vk::PhysicalDevice>& physicalDevices = _instance.enumeratePhysicalDevices();
    engine::RankedPhysicalDeviceStrategy physicalDeviceStrategy{};
    _physicalDevice = physicalDeviceStrategy.pickPhysicalDevice(physicalDevices);

    _surface = _window.createSurface(_instance);
    std::pair<uint32_t, uint32_t> queueFamilyIndices =
        engine::findGraphicsAndPresentQueueFamilyIndex(_physicalDevice, _surface);
    _graphicsQueueIndex = queueFamilyIndices.first;
    _presentQueueIndex = queueFamilyIndices.second;
    std::println("Physical device created...");

    _device = engine::DeviceBuilder(_physicalDevice, _graphicsQueueIndex)
        .setExtensions({
            VK_KHR_SWAPCHAIN_EXTENSION_NAME, 
            //VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME
        })
        //.setPNext(new vk::PhysicalDeviceDynamicRenderingFeatures(VK_TRUE))
        .build();

    std::println("Device created...");

    _descriptorSetLayout =
        engine::DescriptorSetLayoutBuilder(_device)
            .addBinding(vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex)
            .addBinding(
                vk::DescriptorType::eCombinedImageSampler,
                1,
                vk::ShaderStageFlagBits::eFragment)
            .build(_descriptorSetLayoutBindings);

    vk::PushConstantRange pushConstantRange{};
    pushConstantRange.setStageFlags(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);
    pushConstantRange.setOffset(0);
    pushConstantRange.setSize(sizeof(engine::SimplePushConstantData));

    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts{_descriptorSetLayout};

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.setSetLayouts(descriptorSetLayouts);
    pipelineLayoutInfo.setPushConstantRanges(pushConstantRange);
    _pipelineLayout = _device.createPipelineLayout(pipelineLayoutInfo);
}


void DynamicApp::load() {

}