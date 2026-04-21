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

RaiiApp::RaiiApp() {}

RaiiApp::~RaiiApp() {}

void RaiiApp::run() {
  try {
    char const* AppName = "DrawTexturedCube";
    char const* EngineName = "Vulkan.hpp";

    engine::Window window(AppName, 500, 500);
    std::vector<std::string> extensions = window.getRequiredExtensions();

    for (int i = 0; i < extensions.size(); i++) {
        std::cout << "Extension required: " << extensions[i] << std::endl; 
    }

    vk::Instance instance = engine::InstanceBuilder(AppName, EngineName, VK_API_VERSION_1_3)
                                .setExtensions(extensions)
                                .build();

    window.createSurface(instance);

#if !defined(NDEBUG)
    // TODO: Do this in the InstanceBuilder instead
    // vk::DebugUtilsMessengerEXT debugUtilsMessenger = instance.createDebugUtilsMessengerEXT(
    // vk::su::makeDebugUtilsMessengerCreateInfoEXT() );
#endif

    const std::vector<vk::PhysicalDevice>& physicalDevices = instance.enumeratePhysicalDevices();
    engine::PhysicalDeviceStrategy* physicalDeviceStrategy = new engine::RankedPhysicalDeviceStrategy();
    vk::PhysicalDevice physicalDevice = physicalDeviceStrategy->pickPhysicalDevice(physicalDevices);
    delete physicalDeviceStrategy;

    std::pair<uint32_t, uint32_t> queueFamilyIndices =
        engine::findGraphicsAndPresentQueueFamilyIndex(physicalDevice, window.getSurface());
    uint32_t graphicsQueueIndex = queueFamilyIndices.first;
    uint32_t presentQueueIndex = queueFamilyIndices.second;


    vk::Device device = engine::DeviceBuilder(physicalDevice, graphicsQueueIndex)
                            .setExtensions({
                                VK_KHR_SWAPCHAIN_EXTENSION_NAME, 
                                VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME
                            })
                            .setPNext(new vk::PhysicalDeviceDynamicRenderingFeatures(VK_TRUE))
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
            .addBinding(
                vk::DescriptorType::eCombinedImageSampler,
                1,
                vk::ShaderStageFlagBits::eFragment)
            .build();

    vk::PipelineLayout pipelineLayout = device.createPipelineLayout(
        vk::PipelineLayoutCreateInfo(vk::PipelineLayoutCreateFlags(), descriptorSetLayout));

    // TODO: Abstract into a builder pattern like InstanceBuilder for the vk::Instance
    vk::RenderPass renderPass = vk::su::createRenderPass(
        device,
        vk::su::pickSurfaceFormat(physicalDevice.getSurfaceFormatsKHR(window.getSurface())).format,
        depthBufferData.getFormat());

    std::string vertShaderGlsl = engine::readFileString("../shaders/vertexShaderText_PT_T.vert");
    std::string fragShaderGlsl = engine::readFileString("../shaders/fragmentShaderText_T_C.frag");
    vk::ShaderModule vertexShaderModule =
        engine::ShaderModuleBuilder(device, vk::ShaderStageFlagBits::eVertex, vertShaderGlsl)
            .build();
    vk::ShaderModule fragmentShaderModule =
        engine::ShaderModuleBuilder(device, vk::ShaderStageFlagBits::eFragment, fragShaderGlsl)
            .build();

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

    //engine::Renderer renderer(window, device);

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
    

    bool enableDynamicRendering = true;

    vk::ImageSubresourceRange range{};
    range.aspectMask     = vk::ImageAspectFlagBits::eColor;
    range.baseMipLevel   = 0;
    range.levelCount     = VK_REMAINING_MIP_LEVELS;
    range.baseArrayLayer = 0;
    range.layerCount     = VK_REMAINING_ARRAY_LAYERS;

    vk::ImageSubresourceRange depthRange{range};
    depthRange.aspectMask = vk::ImageAspectFlagBits::eDepth;

    std::cout << "Pre-render complete" << std::endl;

    if (enableDynamicRendering) {
        std::cout << "Dynamic rendering enabled" << std::endl;
        engine::setImageLayout(
            commandBuffer,
            swapChainData.getImages()[0],
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            vk::AccessFlagBits::eNone,
            vk::AccessFlagBits::eColorAttachmentWrite,
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::eColorAttachmentOptimal,
            range
        );

        engine::setImageLayout(
            commandBuffer,
            depthBufferData.getImage(),
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::eDepthAttachmentOptimal,
            depthRange
        );

        vk::RenderingAttachmentInfoKHR colorAttachmentInfo; 
        colorAttachmentInfo.imageView = swapChainData.getImageViews()[0];
        colorAttachmentInfo.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
        colorAttachmentInfo.resolveMode = vk::ResolveModeFlagBits::eNone;
        colorAttachmentInfo.loadOp = vk::AttachmentLoadOp::eClear;
        colorAttachmentInfo.storeOp = vk::AttachmentStoreOp::eStore;
        colorAttachmentInfo.clearValue = clearValues[0];
        
        vk::RenderingAttachmentInfoKHR depthAttachmentInfo;
        depthAttachmentInfo.imageView = depthBufferData.getImageView();
        depthAttachmentInfo.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal;
        depthAttachmentInfo.resolveMode = vk::ResolveModeFlagBits::eNone;
        depthAttachmentInfo.loadOp = vk::AttachmentLoadOp::eClear;
        depthAttachmentInfo.storeOp = vk::AttachmentStoreOp::eDontCare;
        depthAttachmentInfo.clearValue = clearValues[1];
        
        vk::Rect2D renderArea = vk::Rect2D(vk::Offset2D(0, 0), window.getExtent());
        vk::RenderingInfoKHR renderingInfo   = {};
        renderingInfo.pNext                = VK_NULL_HANDLE;
        renderingInfo.renderArea           = renderArea;
        renderingInfo.layerCount           = 1;
        renderingInfo.viewMask             = 0;
        renderingInfo.setColorAttachments(colorAttachmentInfo);
        renderingInfo.setPDepthAttachment(&depthAttachmentInfo);
        
        /*
        vk::Format depthFormat = depthBufferData.getFormat();
        if (depthFormat == vk::Format::eD16Unorm || depthFormat == vk::Format::eD32Sfloat) {
            renderingInfo.setPStencilAttachment(&depthAttachmentInfo);
        } 
        */

        commandBuffer.beginRendering(renderingInfo);
        // TODO: drawScene();
        commandBuffer.endRendering();
        // TODO: drawUI();


        /*        
        vkCmdBeginRenderingKHR(draw_cmd_buffer, &render_info);
        draw_scene();
        vkCmdEndRenderingKHR(draw_cmd_buffer);

        draw_ui(draw_cmd_buffer, i);

        vkb::image_layout_transition(draw_cmd_buffer,
                                        swapchain_buffers[i].image,
                                        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                                        range);
        */
    } else {

        std::cout << "Dynamic rendering disbled" << std::endl;

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
    }

    commandBuffer.end();
    
    vk::Fence drawFence = device.createFence(vk::FenceCreateInfo());

    vk::PipelineStageFlags waitDestinationStageMask(
        vk::PipelineStageFlagBits::eColorAttachmentOutput);
    vk::SubmitInfo submitInfo(imageAcquiredSemaphore, waitDestinationStageMask, commandBuffer);
    graphicsQueue.submit(submitInfo, drawFence);

    while (vk::Result::eTimeout == device.waitForFences(drawFence, VK_TRUE, vk::su::FenceTimeout));

    // TODO: Use at least two swap chains to resolve the validation error https://www.reddit.com/r/vulkan/comments/16lhh6d/error_pswapchains0_images_passed_to_present_must/
   
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

