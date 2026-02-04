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

#include "./utils/geometries.hpp"
#include "./utils/math.hpp"
#include "./utils/shaders.hpp"
#include "./utils/utils.hpp"
#include "glslang/Public/ShaderLang.h"
#include "buffer_raii.hpp"
#include "image_raii.hpp"

#include "window.hpp"
#include "device.hpp"
#include <thread>

RaiiApp::RaiiApp() {
}

RaiiApp::~RaiiApp() {}

void RaiiApp::run() {
  try
  {
    char const * AppName    = "DrawTexturedCube";
    char const * EngineName = "Vulkan.hpp";

  
    vk::Instance instance = vk::su::createInstance( AppName, EngineName, {}, vk::su::getInstanceExtensions() );
#if !defined( NDEBUG )
    //vk::DebugUtilsMessengerEXT debugUtilsMessenger = instance.createDebugUtilsMessengerEXT( vk::su::makeDebugUtilsMessengerCreateInfoEXT() );
#endif

    vk::PhysicalDevice physicalDevice = instance.enumeratePhysicalDevices().front();

    vk::su::SurfaceData surfaceData( instance, AppName, vk::Extent2D( 500, 500 ) );

    std::pair<uint32_t, uint32_t> graphicsAndPresentQueueFamilyIndex = vk::su::findGraphicsAndPresentQueueFamilyIndex( physicalDevice, surfaceData.surface );
    vk::Device                    device = vk::su::createDevice( physicalDevice, graphicsAndPresentQueueFamilyIndex.first, vk::su::getDeviceExtensions() );

    vk::CommandPool   commandPool = device.createCommandPool( { {}, graphicsAndPresentQueueFamilyIndex.first } );
    vk::CommandBuffer commandBuffer =
      device.allocateCommandBuffers( vk::CommandBufferAllocateInfo( commandPool, vk::CommandBufferLevel::ePrimary, 1 ) ).front();

    vk::Queue graphicsQueue = device.getQueue( graphicsAndPresentQueueFamilyIndex.first, 0 );
    vk::Queue presentQueue  = device.getQueue( graphicsAndPresentQueueFamilyIndex.second, 0 );

    vk::su::SwapChainData swapChainData( physicalDevice,
                                         device,
                                         surfaceData.surface,
                                         surfaceData.extent,
                                         vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc,
                                         {},
                                         graphicsAndPresentQueueFamilyIndex.first,
                                         graphicsAndPresentQueueFamilyIndex.second );


    // Info:: DepthBufferData has been dropped in favour of using ImageData/ImageRaii directly
    //vk::su::DepthBufferData depthBufferData( physicalDevice, device, vk::Format::eD16Unorm, surfaceData.extent );
    ImageRaii depthBufferData( physicalDevice,
                   device,
                   vk::Format::eD16Unorm,
                   surfaceData.extent,
                   vk::ImageTiling::eOptimal,
                   vk::ImageUsageFlagBits::eDepthStencilAttachment,
                   vk::ImageLayout::eUndefined,
                   vk::MemoryPropertyFlagBits::eDeviceLocal,
                   vk::ImageAspectFlagBits::eDepth );

    vk::su::TextureData textureData( physicalDevice, device );

    commandBuffer.begin( vk::CommandBufferBeginInfo() );
    textureData.setImage( device, commandBuffer, vk::su::CheckerboardImageGenerator() );

        
    BufferRaii uniformBufferData(
        physicalDevice, 
        device, 
        sizeof( glm::mat4x4 ), 
        vk::BufferUsageFlagBits::eUniformBuffer
    );
    glm::mat4x4        mvpcMatrix = vk::su::createModelViewProjectionClipMatrix( surfaceData.extent );
    uniformBufferData.write(mvpcMatrix);
   
    vk::DescriptorSetLayout descriptorSetLayout =
      vk::su::createDescriptorSetLayout( device,
                                         { { vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex },
                                           { vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment } } );
    vk::PipelineLayout pipelineLayout = device.createPipelineLayout( vk::PipelineLayoutCreateInfo( vk::PipelineLayoutCreateFlags(), descriptorSetLayout ) );

    vk::RenderPass renderPass = vk::su::createRenderPass(
      device, vk::su::pickSurfaceFormat( physicalDevice.getSurfaceFormatsKHR( surfaceData.surface ) ).format, depthBufferData.getFormat() );

    glslang::InitializeProcess();
    // TODO: Replace with createShaderModule from pipeline.cpp
    // TODO: We need the spv header to be able to pass into pipeline.cpp code
    // "../shaders/vertexShaderText_PT_T.vert.spv",
    // "../shaders/fragmentShaderText_T_C.frag.spv",
    
    auto vertCode = vk::su::readFile("../shaders/vertexShaderText_PT_T.vert.spv");
    auto fragCode = vk::su::readFile("../shaders/fragmentShaderText_T_C.frag.spv");
    
    vk::ShaderModule vertexShaderModule   = vk::su::createPrecompiledShaderModule( device, vk::ShaderStageFlagBits::eVertex, vertCode );
    vk::ShaderModule fragmentShaderModule = vk::su::createPrecompiledShaderModule( device, vk::ShaderStageFlagBits::eFragment, fragCode);
    glslang::FinalizeProcess();

    std::vector<vk::Framebuffer> framebuffers =
      vk::su::createFramebuffers( device, renderPass, swapChainData.imageViews, depthBufferData.getImageView(), surfaceData.extent );

    BufferRaii vertexBufferData(
        physicalDevice, 
        device, 
        sizeof( texturedCubeData ),
        vk::BufferUsageFlagBits::eVertexBuffer
    );
    vertexBufferData.write( texturedCubeData, sizeof( texturedCubeData ) / sizeof( texturedCubeData[0] ) );
    
    vk::DescriptorPool descriptorPool =
      vk::su::createDescriptorPool( device, { { vk::DescriptorType::eUniformBuffer, 1 }, { vk::DescriptorType::eCombinedImageSampler, 1 } } );
    vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo( descriptorPool, descriptorSetLayout );
    vk::DescriptorSet             descriptorSet = device.allocateDescriptorSets( descriptorSetAllocateInfo ).front();

    vk::su::updateDescriptorSets( device, descriptorSet, { { vk::DescriptorType::eUniformBuffer, uniformBufferData.getBuffer(), VK_WHOLE_SIZE, {} } }, textureData );

    vk::PipelineCache pipelineCache    = device.createPipelineCache( vk::PipelineCacheCreateInfo() );
    vk::Pipeline      graphicsPipeline = vk::su::createGraphicsPipeline( device,
                                                                    pipelineCache,
                                                                    std::make_pair( vertexShaderModule, nullptr ),
                                                                    std::make_pair( fragmentShaderModule, nullptr ),
                                                                    sizeof( texturedCubeData[0] ),
                                                                    { { vk::Format::eR32G32B32A32Sfloat, 0 }, { vk::Format::eR32G32Sfloat, 16 } },
                                                                    vk::FrontFace::eClockwise,
                                                                    true,
                                                                    pipelineLayout,
                                                                    renderPass );
    /* VULKAN_KEY_START */

    // Get the index of the next available swapchain image:
    vk::Semaphore             imageAcquiredSemaphore = device.createSemaphore( vk::SemaphoreCreateInfo() );
    vk::ResultValue<uint32_t> currentBuffer = device.acquireNextImageKHR( swapChainData.swapChain, vk::su::FenceTimeout, imageAcquiredSemaphore, nullptr );
    assert( currentBuffer.result == vk::Result::eSuccess );
    assert( currentBuffer.value < framebuffers.size() );

    // commandBuffer.begin() has already been called above!

    std::array<vk::ClearValue, 2> clearValues;
    clearValues[0].color        = vk::ClearColorValue( 0.2f, 0.2f, 0.2f, 0.2f );
    clearValues[1].depthStencil = vk::ClearDepthStencilValue( 1.0f, 0 );
    vk::RenderPassBeginInfo renderPassBeginInfo(
      renderPass, framebuffers[currentBuffer.value], vk::Rect2D( vk::Offset2D( 0, 0 ), surfaceData.extent ), clearValues );
    commandBuffer.beginRenderPass( renderPassBeginInfo, vk::SubpassContents::eInline );
    commandBuffer.bindPipeline( vk::PipelineBindPoint::eGraphics, graphicsPipeline );
    commandBuffer.bindDescriptorSets( vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, descriptorSet, nullptr );

    commandBuffer.bindVertexBuffers( 0, vertexBufferData.getBuffer(), { 0 } );
    commandBuffer.setViewport(
      0, vk::Viewport( 0.0f, 0.0f, static_cast<float>( surfaceData.extent.width ), static_cast<float>( surfaceData.extent.height ), 0.0f, 1.0f ) );
    commandBuffer.setScissor( 0, vk::Rect2D( vk::Offset2D( 0, 0 ), surfaceData.extent ) );

    commandBuffer.draw( 12 * 3, 1, 0, 0 );
    commandBuffer.endRenderPass();
    commandBuffer.end();

    vk::Fence drawFence = device.createFence( vk::FenceCreateInfo() );

    vk::PipelineStageFlags waitDestinationStageMask( vk::PipelineStageFlagBits::eColorAttachmentOutput );
    vk::SubmitInfo         submitInfo( imageAcquiredSemaphore, waitDestinationStageMask, commandBuffer );
    graphicsQueue.submit( submitInfo, drawFence );

    while ( vk::Result::eTimeout == device.waitForFences( drawFence, VK_TRUE, vk::su::FenceTimeout ) )
      ;

    vk::Result result = presentQueue.presentKHR( vk::PresentInfoKHR( {}, swapChainData.swapChain, currentBuffer.value ) );
    switch ( result )
    {
      case vk::Result::eSuccess: break;
      case vk::Result::eSuboptimalKHR: std::cout << "vk::Queue::presentKHR returned vk::Result::eSuboptimalKHR !\n"; break;
      default: assert( false );  // an unexpected result is returned !
    }
    std::this_thread::sleep_for( std::chrono::milliseconds( 1000 ) );

    /* VULKAN_KEY_END */

    device.waitIdle();

    device.destroyFence( drawFence );
    device.destroySemaphore( imageAcquiredSemaphore );
    device.destroyPipeline( graphicsPipeline );
    device.destroyPipelineCache( pipelineCache );
    device.freeDescriptorSets( descriptorPool, descriptorSet );
    device.destroyDescriptorPool( descriptorPool );
    vertexBufferData.clear( );
    for ( auto framebuffer : framebuffers )
    {
      device.destroyFramebuffer( framebuffer );
    }
    device.destroyShaderModule( fragmentShaderModule );
    device.destroyShaderModule( vertexShaderModule );
    device.destroyRenderPass( renderPass );
    device.destroyPipelineLayout( pipelineLayout );
    device.destroyDescriptorSetLayout( descriptorSetLayout );
    uniformBufferData.clear( );
    textureData.clear( device );
    depthBufferData.clear( device );
    swapChainData.clear( device );
    device.freeCommandBuffers( commandPool, commandBuffer );
    device.destroyCommandPool( commandPool );
    device.destroy();
    instance.destroySurfaceKHR( surfaceData.surface );
#if !defined( NDEBUG )
    //instance.destroyDebugUtilsMessengerEXT( debugUtilsMessenger );
#endif
    instance.destroy();
  }
  catch ( vk::SystemError & err )
  {
    std::cout << "vk::SystemError: " << err.what() << std::endl;
    exit( -1 );
  }
  catch ( std::exception & err )
  {
    std::cout << "std::exception: " << err.what() << std::endl;
    exit( -1 );
  }
  catch ( ... )
  {
    std::cout << "unknown error\n";
    exit( -1 );
  }
}
