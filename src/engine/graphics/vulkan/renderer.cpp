#include "renderer.hpp"
#include <print>

namespace engine {

Renderer::Renderer(
    vk::Instance instance,
    engine::Window& window,    
    vk::SurfaceKHR surface,  vk::Device device,
    vk::PhysicalDevice physicalDevice, vk::CommandPool commandPool, 
    uint32_t graphicsFamilyIndex,
    uint32_t presentFamilyIndex,
    uint32_t maxFramesInFlight,
    bool enableDynamicRendering)
    : 
    _instance{instance},
    _window{window},
    _surface{surface}, 
    _device{device}, 
    _physicalDevice{physicalDevice}, 
    _commandPool{commandPool}, 
    _graphicsFamilyIndex{graphicsFamilyIndex},
    _presentFamilyIndex{presentFamilyIndex},
    _maxFramesInFlight{maxFramesInFlight},
    _enableDynamicRendering{enableDynamicRendering} {
  auto extent = _window.getExtent();
  recreateSwapChain(extent);
  createCommandBuffers();

  _window.addFramebufferResizeListener([&] (vk::Extent2D extent) { 
    std::println("Window resized");
    recreateSwapChain(extent);
  });
}

Renderer::~Renderer() { freeCommandBuffers(); }

void Renderer::recreateSwapChain(vk::Extent2D extent) {
  _device.waitIdle();

  std::println("Recreating swap chain {} x {}", extent.width, extent.height);
  
  if (_swapChain == nullptr) {
    _swapChain = std::make_unique<engine::SwapChain>(
      _instance,
      _device,
      _physicalDevice,
      _surface, 
      _window.getExtent(),
      _graphicsFamilyIndex,
      _presentFamilyIndex, 
      _maxFramesInFlight,
      _enableDynamicRendering
    );
  } else {
    std::shared_ptr<engine::SwapChain> oldSwapChain = std::move(_swapChain);
    _swapChain = std::make_unique<engine::SwapChain>(
      _instance,
      _device,
      _physicalDevice,
      _surface,
      _window.getExtent(),
      _graphicsFamilyIndex,
      _presentFamilyIndex, 
      _maxFramesInFlight,
      _enableDynamicRendering,
      oldSwapChain
    );

    if (!oldSwapChain->compareSwapFormats(*_swapChain.get())) {
      throw std::runtime_error("Swap chain image(or depth) format has changed!");
    }
  }
}

void Renderer::transitionImageLayout(
    vk::CommandBuffer                commandBuffer,
    vk::Image                        image,
    vk::PipelineStageFlags           srcStageMask,
    vk::PipelineStageFlags           dstStageMask,
    vk::AccessFlags                  srcAccessMask,
    vk::AccessFlags                  dstAccessMask,
    vk::ImageLayout                  oldLayout,
    vk::ImageLayout                  newLayout,
    vk::ImageSubresourceRange const &subresourceRange) {

	vk::ImageMemoryBarrier imageMemoryBarrier = vk::ImageMemoryBarrier()
      .setSrcAccessMask(srcAccessMask)
      .setDstAccessMask(dstAccessMask)
      .setOldLayout(oldLayout)
      .setNewLayout(newLayout)
      .setSrcQueueFamilyIndex(vk::QueueFamilyIgnored)
      .setDstQueueFamilyIndex(vk::QueueFamilyIgnored)
      .setImage(image)
      .setSubresourceRange(subresourceRange);

  commandBuffer.pipelineBarrier(srcStageMask, dstStageMask, vk::DependencyFlags(), {}, {}, imageMemoryBarrier);
}

void Renderer::transitionImageLayout(
  vk::CommandBuffer                commandBuffer,
  vk::Image                        image,
  vk::ImageLayout                  oldLayout,
  vk::ImageLayout                  newLayout,
  vk::ImageSubresourceRange const &subresourceRange
) {

	vk::PipelineStageFlags srcStageMask  = getPipelineStageFlags(oldLayout);
	vk::PipelineStageFlags dstStageMask  = getPipelineStageFlags(newLayout);
	vk::AccessFlags        srcAccessMask = getAccessFlags(oldLayout);
	vk::AccessFlags        dstAccessMask = getAccessFlags(newLayout);

	transitionImageLayout(commandBuffer, image, srcStageMask, dstStageMask, srcAccessMask, dstAccessMask, oldLayout, newLayout, subresourceRange);
}

vk::PipelineStageFlags Renderer::getPipelineStageFlags(vk::ImageLayout layout)
{
	switch (layout)
	{
		case vk::ImageLayout::eUndefined:
			return vk::PipelineStageFlagBits::eTopOfPipe;
		case vk::ImageLayout::ePreinitialized:
			return vk::PipelineStageFlagBits::eHost;
		case vk::ImageLayout::eTransferDstOptimal:
		case vk::ImageLayout::eTransferSrcOptimal:
			return vk::PipelineStageFlagBits::eTransfer;
		case vk::ImageLayout::eColorAttachmentOptimal:
			return vk::PipelineStageFlagBits::eColorAttachmentOutput;
		case vk::ImageLayout::eDepthAttachmentOptimal:
			return vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests;
		case vk::ImageLayout::eFragmentShadingRateAttachmentOptimalKHR:
			return vk::PipelineStageFlagBits::eFragmentShadingRateAttachmentKHR;
		case vk::ImageLayout::eShaderReadOnlyOptimal:
			return vk::PipelineStageFlagBits::eVertexShader | vk::PipelineStageFlagBits::eFragmentShader;
		case vk::ImageLayout::ePresentSrcKHR:
			return vk::PipelineStageFlagBits::eBottomOfPipe;
		case vk::ImageLayout::eGeneral:
			assert(false && "Don't know how to get a meaningful VkPipelineStageFlags for VK_IMAGE_LAYOUT_GENERAL! Don't use it!");
			return {};
		default:
			assert(false);
			return {};
	}
}

vk::AccessFlags Renderer::getAccessFlags(vk::ImageLayout layout)
{
	switch (layout)
	{
		case vk::ImageLayout::eUndefined:
    case vk::ImageLayout::ePresentSrcKHR:
			return {};
		case vk::ImageLayout::ePreinitialized:
			return vk::AccessFlagBits::eHostWrite;
		case vk::ImageLayout::eColorAttachmentOptimal:
			return vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
		case vk::ImageLayout::eDepthAttachmentOptimal:
			return vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
		case vk::ImageLayout::eFragmentShadingRateAttachmentOptimalKHR:
			return vk::AccessFlagBits::eFragmentShadingRateAttachmentReadKHR;
		case vk::ImageLayout::eShaderReadOnlyOptimal:
			return vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eInputAttachmentRead;
		case vk::ImageLayout::eTransferSrcOptimal:
			return vk::AccessFlagBits::eTransferRead;
		case vk::ImageLayout::eTransferDstOptimal:
			return vk::AccessFlagBits::eTransferWrite;
		case vk::ImageLayout::eGeneral:
			assert(false && "Don't know how to get a meaningful VkAccessFlags for VK_IMAGE_LAYOUT_GENERAL! Don't use it!");
			return {};
		default:
			assert(false);
			return {};
	}
}


void Renderer::createCommandBuffers() {
  _commandBuffers.resize(_maxFramesInFlight);

  vk::CommandBufferAllocateInfo allocInfo = vk::CommandBufferAllocateInfo()
    .setLevel(vk::CommandBufferLevel::ePrimary)
    .setCommandPool(_commandPool)
    .setCommandBufferCount(_commandBuffers.size());

  _commandBuffers = _device.allocateCommandBuffers(allocInfo);
}

void Renderer::freeCommandBuffers() {
  _device.freeCommandBuffers(_commandPool, _commandBuffers);
  _commandBuffers.clear();
}

bool Renderer::renderFrame(
    std::function<void(vk::CommandBuffer const &, int)> preRender, 
    std::initializer_list<std::function<void(vk::CommandBuffer const &, int)>> renderOperations,
    std::function<void(vk::CommandBuffer const &, int)> postRender
  ) {
  auto acquireNextImageResult = _swapChain->acquireNextImage(&_currentImageIndex);
  if (acquireNextImageResult == vk::Result::eErrorOutOfDateKHR) {
    std::println("Out of date KHR after acquiring next image");
    recreateSwapChain(_window.getExtent());
    return false;
  }

  if (acquireNextImageResult != vk::Result::eSuccess && acquireNextImageResult != vk::Result::eSuboptimalKHR) {
    throw std::runtime_error("failed to acquire swap chain image!");
  }

  auto commandBuffer = _commandBuffers[_currentFrameIndex];
  vk::CommandBufferBeginInfo beginInfo = vk::CommandBufferBeginInfo();
  commandBuffer.begin(beginInfo);
  preRender(commandBuffer, _currentFrameIndex);

  if (_enableDynamicRendering) {    
    _swapChain->run(commandBuffer, _currentFrameIndex, _currentImageIndex, [&] (vk::CommandBuffer const & commandBuffer, int frameIndex) {
      for( auto renderPassOperation : renderOperations ) {  
        renderPassOperation(commandBuffer, frameIndex);
      }
    });
  } else {
    for( auto renderPassOperation : renderOperations )
    {
      _swapChain->runRenderPass(commandBuffer, _currentFrameIndex, _currentImageIndex, renderPassOperation);
    }
  }
  
  postRender(commandBuffer, _currentFrameIndex);
  commandBuffer.end();

  auto submitCommandBuffersResult = _swapChain->submitCommandBuffers(&commandBuffer, &_currentImageIndex);
  if (submitCommandBuffersResult == vk::Result::eErrorOutOfDateKHR || submitCommandBuffersResult == vk::Result::eSuboptimalKHR) {
    std::println("Error after submitting command buffers");
    recreateSwapChain(_window.getExtent());
  } else if (submitCommandBuffersResult != vk::Result::eSuccess) {
    throw std::runtime_error("failed to present swap chain image!");
  }

  _currentFrameIndex = (_currentFrameIndex + 1) % _maxFramesInFlight;

  return true;
}

}  // namespace engine

