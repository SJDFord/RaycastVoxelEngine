#include "raii_app.hpp"

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

#include "./engine/buffer.hpp"
#include "./engine/descriptor_set_layout_builder.hpp"
#include "./engine/descriptor_set_utils.hpp"
#include "./engine/device_builder.hpp"
#include "./engine/image.hpp"
#include "./engine/image_generators.hpp"
#include "./engine/instance_builder.hpp"
#include "./engine/physical_device_strategy.hpp"
#include "./engine/ranked_physical_device_strategy.hpp"
#include "./engine/swap_chain.hpp"
#include "./engine/texture.hpp"
#include "./engine/utils.hpp"
#include "./engine/window.hpp"
#include "./utils/geometries.hpp"
#include "./utils/math.hpp"
#include "./utils/shaders.hpp"
#include "./utils/utils.hpp"
#include "device.hpp"
#include "glslang/Public/ShaderLang.h"
#include "window.hpp"

RaiiApp::RaiiApp() {}

RaiiApp::~RaiiApp() {}

void RaiiApp::run() {
  try {
    char const* AppName = "DrawTexturedCube";
    char const* EngineName = "Vulkan.hpp";

    std::vector<std::string> extensions;
    extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    extensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_METAL_EXT)
    extensions.push_back(VK_EXT_METAL_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_VI_NN)
    extensions.push_back(VK_NN_VI_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
    extensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
    extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
    extensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
    std::cout << "Requesting ext" << std::endl;
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
    extensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
    extensions.push_back(VK_EXT_ACQUIRE_XLIB_DISPLAY_EXTENSION_NAME);
#endif

    // TODO: Fix properly
    extensions.push_back("VK_KHR_xcb_surface");

    vk::Instance instance = engine::InstanceBuilder(AppName, EngineName, VK_API_VERSION_1_0)
                                .setExtensions(extensions)
                                .build();

#if !defined(NDEBUG)
    // TODO: Do this in the InstanceBuilder instead
    // vk::DebugUtilsMessengerEXT debugUtilsMessenger = instance.createDebugUtilsMessengerEXT(
    // vk::su::makeDebugUtilsMessengerCreateInfoEXT() );
#endif

    const std::vector<vk::PhysicalDevice>& physicalDevices = instance.enumeratePhysicalDevices();
    engine::PhysicalDeviceStrategy physicalDeviceStrategy = engine::RankedPhysicalDeviceStrategy();
    vk::PhysicalDevice physicalDevice = physicalDeviceStrategy.pickPhysicalDevice(physicalDevices);

    engine::Window window(instance, AppName, 500, 500);

    std::pair<uint32_t, uint32_t> queueFamilyIndices =
        engine::findGraphicsAndPresentQueueFamilyIndex(physicalDevice, window.getSurface());
    uint32_t graphicsQueueIndex = queueFamilyIndices.first;
    uint32_t presentQueueIndex = queueFamilyIndices.second;

    vk::Device device = engine::DeviceBuilder(physicalDevice, graphicsQueueIndex)
                            .setExtensions({VK_KHR_SWAPCHAIN_EXTENSION_NAME})
                            .build();

    vk::CommandPool commandPool = device.createCommandPool({{}, graphicsQueueIndex});
    vk::CommandBuffer commandBuffer =
        device
            .allocateCommandBuffers(
                vk::CommandBufferAllocateInfo(commandPool, vk::CommandBufferLevel::ePrimary, 1))
            .front();

    vk::Queue graphicsQueue = device.getQueue(graphicsQueueIndex, 0);
    vk::Queue presentQueue = device.getQueue(presentQueueIndex, 0);

    // TODO: Maybe a builder for this?
    engine::SwapChain swapChainData(
        physicalDevice,
        device,
        window.getSurface(),
        window.getExtent(),
        vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc,
        {},
        graphicsQueueIndex,
        presentQueueIndex);

    // TODO: Abstract into a builder pattern like InstanceBuilder for the vk::Instance
    engine::Image depthBufferData(
        physicalDevice,
        device,
        vk::Format::eD16Unorm,
        window.getExtent(),
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eDepthStencilAttachment,
        vk::ImageLayout::eUndefined,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        vk::ImageAspectFlagBits::eDepth);

    engine::Texture textureData(physicalDevice, device);

    commandBuffer.begin(vk::CommandBufferBeginInfo());
    textureData.setImage(device, commandBuffer, engine::CheckerboardImageGenerator());

    // TODO: Abstract into a builder pattern like InstanceBuilder for the vk::Instance
    engine::Buffer uniformBufferData(
        physicalDevice,
        device,
        sizeof(glm::mat4x4),
        vk::BufferUsageFlagBits::eUniformBuffer);
    glm::mat4x4 mvpcMatrix = vk::su::createModelViewProjectionClipMatrix(window.getExtent());
    uniformBufferData.write(mvpcMatrix);

    vk::DescriptorSetLayout descriptorSetLayout =
        engine::DescriptorSetLayoutBuilder(device)
            .addBinding(vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex)
            .addBinding(vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment)
            .build();

    vk::PipelineLayout pipelineLayout = device.createPipelineLayout(
        vk::PipelineLayoutCreateInfo(vk::PipelineLayoutCreateFlags(), descriptorSetLayout));

    // TODO: Abstract into a builder pattern like InstanceBuilder for the vk::Instance
    vk::RenderPass renderPass = vk::su::createRenderPass(
        device,
        vk::su::pickSurfaceFormat(physicalDevice.getSurfaceFormatsKHR(window.getSurface())).format,
        depthBufferData.getFormat());

    glslang::InitializeProcess();
    // TODO: Compile the GLSL shader to spv during runtime (and cache the result) rather than at
    // compile time. This is because the compiled shaders are system specific but the GLSL is
    // universal
    // "../shaders/vertexShaderText_PT_T.vert.spv",
    // "../shaders/fragmentShaderText_T_C.frag.spv",

    auto vertCode = vk::su::readFile("../shaders/vertexShaderText_PT_T.vert.spv");
    auto fragCode = vk::su::readFile("../shaders/fragmentShaderText_T_C.frag.spv");

    // TODO: Builder pattern?
    vk::ShaderModule vertexShaderModule =
        vk::su::createPrecompiledShaderModule(device, vk::ShaderStageFlagBits::eVertex, vertCode);
    vk::ShaderModule fragmentShaderModule =
        vk::su::createPrecompiledShaderModule(device, vk::ShaderStageFlagBits::eFragment, fragCode);
    glslang::FinalizeProcess();

    // TODO: Builder pattern?
    std::vector<vk::Framebuffer> framebuffers = vk::su::createFramebuffers(
        device,
        renderPass,
        swapChainData.getImageViews(),
        depthBufferData.getImageView(),
        window.getExtent());

    // TODO: Builder pattern?
    engine::Buffer vertexBufferData(
        physicalDevice,
        device,
        sizeof(texturedCubeData),
        vk::BufferUsageFlagBits::eVertexBuffer);
    vertexBufferData.write(
        texturedCubeData,
        sizeof(texturedCubeData) / sizeof(texturedCubeData[0]));

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
        renderPass);
    /* VULKAN_KEY_START */

    // Get the index of the next available swapchain image:
    vk::Semaphore imageAcquiredSemaphore = device.createSemaphore(vk::SemaphoreCreateInfo());
    vk::ResultValue<uint32_t> currentBuffer = device.acquireNextImageKHR(
        swapChainData.getSwapChain(),
        vk::su::FenceTimeout,
        imageAcquiredSemaphore,
        nullptr);
    assert(currentBuffer.result == vk::Result::eSuccess);
    assert(currentBuffer.value < framebuffers.size());

    // commandBuffer.begin() has already been called above!

    std::array<vk::ClearValue, 2> clearValues;
    clearValues[0].color = vk::ClearColorValue(0.2f, 0.2f, 0.2f, 0.2f);
    clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);
    vk::RenderPassBeginInfo renderPassBeginInfo(
        renderPass,
        framebuffers[currentBuffer.value],
        vk::Rect2D(vk::Offset2D(0, 0), window.getExtent()),
        clearValues);
    commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);
    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics,
        pipelineLayout,
        0,
        descriptorSet,
        nullptr);

    commandBuffer.bindVertexBuffers(0, vertexBufferData.getBuffer(), {0});
    commandBuffer.setViewport(
        0,
        vk::Viewport(
            0.0f,
            0.0f,
            static_cast<float>(window.getExtent().width),
            static_cast<float>(window.getExtent().height),
            0.0f,
            1.0f));
    commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), window.getExtent()));

    commandBuffer.draw(12 * 3, 1, 0, 0);
    commandBuffer.endRenderPass();
    commandBuffer.end();

    vk::Fence drawFence = device.createFence(vk::FenceCreateInfo());

    vk::PipelineStageFlags waitDestinationStageMask(
        vk::PipelineStageFlagBits::eColorAttachmentOutput);
    vk::SubmitInfo submitInfo(imageAcquiredSemaphore, waitDestinationStageMask, commandBuffer);
    graphicsQueue.submit(submitInfo, drawFence);

    while (vk::Result::eTimeout == device.waitForFences(drawFence, VK_TRUE, vk::su::FenceTimeout));

    vk::Result result = presentQueue.presentKHR(
        vk::PresentInfoKHR({}, swapChainData.getSwapChain(), currentBuffer.value));
    switch (result) {
      case vk::Result::eSuccess:
        break;
      case vk::Result::eSuboptimalKHR:
        std::cout << "vk::Queue::presentKHR returned vk::Result::eSuboptimalKHR !\n";
        break;
      default:
        assert(false);  // an unexpected result is returned !
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    /* VULKAN_KEY_END */

    device.waitIdle();

    // TODO: See which elements of this can be moved to the relevant destructors (RAII)
    device.destroyFence(drawFence);
    device.destroySemaphore(imageAcquiredSemaphore);
    device.destroyPipeline(graphicsPipeline);
    device.destroyPipelineCache(pipelineCache);
    device.freeDescriptorSets(descriptorPool, descriptorSet);
    device.destroyDescriptorPool(descriptorPool);
    vertexBufferData.clear(device);
    for (auto framebuffer : framebuffers) {
      device.destroyFramebuffer(framebuffer);
    }
    device.destroyShaderModule(fragmentShaderModule);
    device.destroyShaderModule(vertexShaderModule);
    device.destroyRenderPass(renderPass);
    device.destroyPipelineLayout(pipelineLayout);
    device.destroyDescriptorSetLayout(descriptorSetLayout);
    uniformBufferData.clear(device);
    textureData.clear(device);
    depthBufferData.clear(device);
    swapChainData.clear(device);
    device.freeCommandBuffers(commandPool, commandBuffer);
    device.destroyCommandPool(commandPool);
    device.destroy();
    instance.destroySurfaceKHR(window.getSurface());
#if !defined(NDEBUG)
    // instance.destroyDebugUtilsMessengerEXT( debugUtilsMessenger );
#endif
    instance.destroy();
  } catch (vk::SystemError& err) {
    std::cout << "vk::SystemError: " << err.what() << std::endl;
    exit(-1);
  } catch (std::exception& err) {
    std::cout << "std::exception: " << err.what() << std::endl;
    exit(-1);
  } catch (...) {
    std::cout << "unknown error\n";
    exit(-1);
  }
}

