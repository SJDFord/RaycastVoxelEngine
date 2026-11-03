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

  if (_enableDynamicRendering) {
    vkCmdBeginRenderingKHR = reinterpret_cast<PFN_vkCmdBeginRenderingKHR>(vkGetInstanceProcAddr(_instance, "vkCmdBeginRenderingKHR"));
		vkCmdEndRenderingKHR   = reinterpret_cast<PFN_vkCmdEndRenderingKHR>(vkGetInstanceProcAddr(_instance, "vkCmdEndRenderingKHR"));
		if (!vkCmdBeginRenderingKHR || !vkCmdEndRenderingKHR)
		{
			throw std::runtime_error("Unable to dynamically load vkCmdBeginRenderingKHR and vkCmdEndRenderingKHR");
		}
  }

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
    std::initializer_list<std::function<void(vk::CommandBuffer const &, int)>> renderPassOperations,
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

  if (_enableDynamicRendering) {

    std::array<vk::ClearValue, 2> clearValues{};
    clearValues[0].color        = vk::ClearColorValue(1.0f, 0.0f, 0.0f, 0.0f);
    clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);

		vk::ImageSubresourceRange colorRange = vk::ImageSubresourceRange()
      .setAspectMask(vk::ImageAspectFlagBits::eColor)
      .setBaseMipLevel(0)
      .setLevelCount(vk::RemainingMipLevels)
      .setBaseArrayLayer(0)
      .setLayerCount(vk::RemainingArrayLayers);

		vk::ImageSubresourceRange depthRange = vk::ImageSubresourceRange()
      .setAspectMask(vk::ImageAspectFlagBits::eDepth);

    transitionImageLayout(
      commandBuffer,
      _swapChain->getColorImage(_currentImageIndex),
      vk::PipelineStageFlagBits::eColorAttachmentOutput,
      vk::PipelineStageFlagBits::eColorAttachmentOutput,
      {},
      vk::AccessFlagBits::eColorAttachmentWrite,
      vk::ImageLayout::eUndefined,
      vk::ImageLayout::eColorAttachmentOptimal,
      colorRange
    );

    transitionImageLayout(
      commandBuffer,                      
      _swapChain->getDepthImage(_currentImageIndex),
      vk::ImageLayout::eUndefined,
      vk::ImageLayout::eDepthAttachmentOptimal,
      depthRange
    );

    vk::RenderingAttachmentInfoKHR colorAttachmentInfo = vk::RenderingAttachmentInfoKHR()
      .setImageView(_swapChain->getColorImageView(_currentImageIndex))
      .setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
      .setResolveMode(vk::ResolveModeFlagBits::eNone)
      .setLoadOp(vk::AttachmentLoadOp::eClear)
      .setStoreOp(vk::AttachmentStoreOp::eStore)
      .setClearValue(clearValues[0]);

    vk::RenderingAttachmentInfoKHR depthAttachmentInfo = vk::RenderingAttachmentInfoKHR()
      .setImageView(_swapChain->getDepthImageView(_currentImageIndex))
      .setImageLayout(vk::ImageLayout::eDepthAttachmentOptimal)
      .setResolveMode(vk::ResolveModeFlagBits::eNone)
      .setLoadOp(vk::AttachmentLoadOp::eClear)
      .setStoreOp(vk::AttachmentStoreOp::eDontCare)
      .setClearValue(clearValues[1]);

    auto extent = _swapChain->getExtent();
    auto renderArea             = vk::Rect2D{vk::Offset2D{}, extent};
    auto renderInfo             = vk::RenderingInfoKHR()
      .setRenderArea(renderArea)
      .setColorAttachmentCount(1)
      .setPColorAttachments(&colorAttachmentInfo)
      .setLayerCount(1)
      .setPDepthAttachment(&depthAttachmentInfo);

    auto depthFormat = _swapChain->getDepthFormat();
    if (depthFormat != vk::Format::eD16Unorm && depthFormat != vk::Format::eD32Sfloat)
    {
      renderInfo.setPStencilAttachment(&depthAttachmentInfo);
    }


    vkCmdBeginRenderingKHR(static_cast<VkCommandBuffer>(commandBuffer), reinterpret_cast<const VkRenderingInfo *>(&renderInfo));
    //commandBuffer.beginRenderingKHR(&renderInfo);

    //vkCmdBeginRenderingKHR(draw_cmd_buffer, &render_info);
    //draw_scene();


    preRender(commandBuffer, _currentFrameIndex);

    vk::Viewport viewport = vk::Viewport()
      .setX(0.0f)
      .setY(0.0f)
      .setWidth(static_cast<float>(extent.width))
      .setHeight(static_cast<float>(extent.height))
      .setMinDepth(0.0f)
      .setMaxDepth(1.0f);
    vk::Rect2D scissor{{0, 0}, extent};
    commandBuffer.setViewport(0, viewport);
    commandBuffer.setScissor(0, scissor);

    for( auto renderPassOperation : renderPassOperations ) {  
      renderPassOperation(commandBuffer, _currentFrameIndex);
    }

    postRender(commandBuffer, _currentFrameIndex);

    
    //commandBuffer.endRenderingKHR();
    //vkCmdEndRenderingKHR(commandBuffer);
    vkCmdEndRenderingKHR(static_cast<VkCommandBuffer>(commandBuffer));

    transitionImageLayout(
      commandBuffer,
      _swapChain->getColorImage(_currentImageIndex),
      vk::ImageLayout::eColorAttachmentOptimal,
      vk::ImageLayout::ePresentSrcKHR,
      colorRange
    );
  } else {
    preRender(commandBuffer, _currentFrameIndex);

    for( auto renderPassOperation : renderPassOperations )
    {
      _swapChain->runRenderPass(commandBuffer, _currentFrameIndex, _currentImageIndex, renderPassOperation);
    }

    postRender(commandBuffer, _currentFrameIndex);
  }

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

