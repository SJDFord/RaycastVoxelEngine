#include "swap_chain.hpp"

#include "./builders/swapchain_builder.hpp"
#include "./builders/render_pass_builder.hpp"

#include <algorithm>
#include <print>

namespace engine {

SwapChain::SwapChain(
    vk::Device         device,
    vk::PhysicalDevice physicalDevice,
    vk::SurfaceKHR     surface,
    vk::Extent2D       windowExtent,
uint32_t                   graphicsFamilyIndex,
                     uint32_t                   presentFamilyIndex,
                     uint32_t                   maxFramesInFlight,
    std::shared_ptr<SwapChain> previous)
    : _device{device}, _physicalDevice{physicalDevice}, _graphicsFamilyIndex{graphicsFamilyIndex}, _presentFamilyIndex{presentFamilyIndex}, _maxFramesInFlight(maxFramesInFlight), _oldSwapChain{previous} {
  init(surface, windowExtent);
  _oldSwapChain = nullptr;
}

void SwapChain::init(vk::SurfaceKHR surface, vk::Extent2D windowExtent) {
  createSwapChain(surface, windowExtent);
  createImageViews();
  createRenderPass();
  createDepthResources();
  createFramebuffers();
  createSyncObjects();
}

SwapChain::~SwapChain() {
  for (auto imageView : _colorImageViews) {
    _device.destroyImageView(imageView); 
  }
  _colorImageViews.clear();

  if (_swapChain != nullptr) {
    _device.destroySwapchainKHR(_swapChain);
    _swapChain = nullptr;
  }

  for (int i = 0; i < _depthImages.size(); i++) {
    _device.destroyImageView(_depthImageViews[i]);
    _device.destroyImage(_depthImages[i]);
    _device.freeMemory(_depthImageMemorys[i]);
  }

  for (auto framebuffer : _framebuffers) {
    _device.destroyFramebuffer(framebuffer);
  }

  _device.destroyRenderPass(_renderPass);

  // cleanup synchronization objects
  for (size_t i = 0; i < _maxFramesInFlight; i++) {
    _device.destroySemaphore(_renderFinishedSemaphores[i]);
    _device.destroySemaphore(_imageAvailableSemaphores[i]);
    _device.destroyFence(_inFlightFences[i]);
  }
}

vk::Result SwapChain::acquireNextImage(uint32_t *imageIndex) {
  _device.waitForFences(_inFlightFences[_currentFrame], true, std::numeric_limits<uint64_t>::max());
  return _device.acquireNextImageKHR(_swapChain, std::numeric_limits<uint64_t>::max(), _imageAvailableSemaphores[_currentFrame], VK_NULL_HANDLE, imageIndex);
}

vk::Result SwapChain::submitCommandBuffers(const vk::CommandBuffer *buffers, uint32_t *imageIndex) {
  if (_imagesInFlight[*imageIndex] != VK_NULL_HANDLE) {
    _device.waitForFences(_imagesInFlight[*imageIndex], true, UINT64_MAX);
  }
  _imagesInFlight[*imageIndex] = _inFlightFences[_currentFrame];

  vk::PipelineStageFlags stageFlags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
  vk::SubmitInfo submitInfo = vk::SubmitInfo()
    .setWaitSemaphores(_imageAvailableSemaphores[_currentFrame])
    .setWaitDstStageMask(stageFlags)
    .setCommandBufferCount(1)
    .setPCommandBuffers(buffers)
    .setSignalSemaphores(_renderFinishedSemaphores[_currentFrame]);
 
  _device.resetFences(_inFlightFences[_currentFrame]);
  _device.getQueue(_graphicsFamilyIndex, 0).submit(submitInfo, _inFlightFences[_currentFrame]);
  
  vk::PresentInfoKHR presentInfo = vk::PresentInfoKHR()
    .setWaitSemaphores(_renderFinishedSemaphores[_currentFrame])
    .setSwapchains(_swapChain)
    .setPImageIndices(imageIndex);

  auto result = _device.getQueue(_presentFamilyIndex, 0).presentKHR(presentInfo);

  _currentFrame = (_currentFrame + 1) % _maxFramesInFlight;

  return result;
}

void SwapChain::runRenderPass(const vk::CommandBuffer& commandBuffer, uint32_t frameIndex, uint32_t imageIndex, std::function<void(vk::CommandBuffer const &, int)> renderPassOperation) {
 vk::Rect2D renderArea = vk::Rect2D({0, 0}, _extent);

  std::array<vk::ClearValue, 2> clearValues{};
  clearValues[0].color = vk::ClearColorValue(0.01f, 0.01f, 1.0f, 1.0f);
  clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);

  vk::RenderPassBeginInfo renderPassInfo = vk::RenderPassBeginInfo()
    .setRenderPass(_renderPass)
    .setFramebuffer(_framebuffers[imageIndex])
    .setRenderArea(renderArea)
    .setClearValues(clearValues);

  commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

  vk::Viewport viewport = vk::Viewport()
    .setX(0.0f)
    .setY(0.0f)
    .setWidth(static_cast<float>(_extent.width))
    .setHeight(static_cast<float>(_extent.height))
    .setMinDepth(0.0f)
    .setMaxDepth(1.0f);
  vk::Rect2D scissor{{0, 0}, _extent};
  commandBuffer.setViewport(0, viewport);
  commandBuffer.setScissor(0, scissor);

  renderPassOperation(commandBuffer, frameIndex);

  commandBuffer.endRenderPass();
}

void SwapChain::createSwapChain(vk::SurfaceKHR surface, vk::Extent2D windowExtent) {
  auto capabilities = _physicalDevice.getSurfaceCapabilitiesKHR(surface);
  auto formats = _physicalDevice.getSurfaceFormatsKHR(surface);
  auto presentModes = _physicalDevice.getSurfacePresentModesKHR(surface);
  vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(formats);
  vk::PresentModeKHR presentMode = chooseSwapPresentMode(presentModes);
  vk::Extent2D extent = chooseSwapExtent(capabilities, windowExtent);

  _swapChain = engine::SwapchainBuilder(_device)
    .setSurfaceCapabilities(capabilities)
    .setSurface(surface)
    .setSurfaceFormat(surfaceFormat)
    .setExtent(extent)
    .setGraphicsQueueFamilyIndex(_graphicsFamilyIndex)
    .setPresentQueueFamilyIndex(_presentFamilyIndex)
    .setPresentMode(presentMode)
    .setOldSwapchain(_oldSwapChain == nullptr ? VK_NULL_HANDLE : _oldSwapChain->_swapChain)
    .build();

  // we only specified a minimum number of images in the swap chain, so the implementation is
  // allowed to create a swap chain with more. That's why we'll first query the final number of
  // images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
  // retrieve the handles.
  
  _colorImages = _device.getSwapchainImagesKHR(_swapChain);
  _colorFormat = surfaceFormat.format;
  _extent = extent;
}

void SwapChain::createImageViews() {
  auto imageCount = _colorImages.size();
  _colorImageViews.resize(imageCount);
  for (size_t i = 0; i < imageCount; i++) {
      // TODO: Image view builder
      vk::ImageSubresourceRange subresourceRange = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor)
        .setBaseMipLevel(0)
        .setLevelCount(1)
        .setBaseArrayLayer(0)
        .setLayerCount(1);

      vk::ImageViewCreateInfo viewInfo = vk::ImageViewCreateInfo()
        .setImage(_colorImages[i])
        .setViewType(vk::ImageViewType::e2D)
        .setFormat(_colorFormat)
        .setSubresourceRange(subresourceRange);

      _colorImageViews[i] = _device.createImageView(viewInfo);
  }
}

void SwapChain::createRenderPass() {
  _renderPass = engine::RenderPassBuilder(_device)
    .setColorFormat(_colorFormat)
    .setDepthFormat(findDepthFormat())
    .build();
}

void SwapChain::createFramebuffers() {
  auto imageCount = _colorImages.size();
  _framebuffers.resize(imageCount);
  for (size_t i = 0; i < imageCount; i++) {
    std::array<vk::ImageView, 2> attachments = {_colorImageViews[i], _depthImageViews[i]};

     // TODO: Framebuffer builder

    vk::FramebufferCreateInfo framebufferInfo = vk::FramebufferCreateInfo()
      .setRenderPass(_renderPass)
      .setAttachments(attachments)
      .setWidth(_extent.width)
      .setHeight(_extent.height)
      .setLayers(1);

    _framebuffers[i] = _device.createFramebuffer(framebufferInfo);
  }
}

void SwapChain::createDepthResources() {
  vk::Format depthFormat = findDepthFormat();
  _depthFormat = depthFormat;
  //VkExtent2D swapChainExtent = getSwapChainExtent();

  auto imageCount = _colorImages.size();
  _depthImages.resize(imageCount);
  _depthImageMemorys.resize(imageCount);
  _depthImageViews.resize(imageCount);

  for (int i = 0; i < _depthImages.size(); i++) {
    // Todo: Image builder
    vk::ImageCreateInfo imageInfo = vk::ImageCreateInfo()
      .setImageType(vk::ImageType::e2D)
      .setExtent(vk::Extent3D(_extent, 1))
      .setMipLevels(1)
      .setArrayLayers(1)
      .setFormat(depthFormat)
      .setTiling(vk::ImageTiling::eOptimal)
      .setInitialLayout(vk::ImageLayout::eUndefined)
      .setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment)
      .setSamples(vk::SampleCountFlagBits::e1)
      .setSharingMode(vk::SharingMode::eExclusive);

    _depthImages[i] = _device.createImage(imageInfo);
    vk::MemoryRequirements memRequirements = _device.getImageMemoryRequirements(_depthImages[i]);

    uint32_t memoryTypeIndex = engine::findMemoryType(
      _physicalDevice.getMemoryProperties(),
      memRequirements.memoryTypeBits, 
      vk::MemoryPropertyFlagBits::eDeviceLocal
    ); 

    vk::MemoryAllocateInfo allocInfo = vk::MemoryAllocateInfo()
      .setAllocationSize(memRequirements.size)
      .setMemoryTypeIndex(memoryTypeIndex);

    _depthImageMemorys[i] = _device.allocateMemory(allocInfo);

    _device.bindImageMemory(_depthImages[i], _depthImageMemorys[i], 0);

    // Todo: Image view builder
      vk::ImageSubresourceRange subresourceRange = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth)
      .setBaseMipLevel(0)
      .setLevelCount(1)
      .setBaseArrayLayer(0)
      .setLayerCount(1);

    vk::ImageViewCreateInfo viewInfo = vk::ImageViewCreateInfo()
      .setImage(_depthImages[i])
      .setViewType(vk::ImageViewType::e2D)
      .setFormat(depthFormat)
      .setSubresourceRange(subresourceRange);

    _depthImageViews[i] = _device.createImageView(viewInfo);
  }
}

void SwapChain::createSyncObjects() {
  _imageAvailableSemaphores.resize(_maxFramesInFlight);
  _renderFinishedSemaphores.resize(_maxFramesInFlight);
  _inFlightFences.resize(_maxFramesInFlight);
  _imagesInFlight.resize(_colorImages.size(), VK_NULL_HANDLE);

  vk::SemaphoreCreateInfo semaphoreInfo = vk::SemaphoreCreateInfo();
  vk::FenceCreateInfo fenceInfo = vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled);

  for (size_t i = 0; i < _maxFramesInFlight; i++) {
    _imageAvailableSemaphores[i] = _device.createSemaphore(semaphoreInfo);
    _renderFinishedSemaphores[i] = _device.createSemaphore(semaphoreInfo);
    _inFlightFences[i] = _device.createFence(fenceInfo);
  }
}

vk::SurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(
    const std::vector<vk::SurfaceFormatKHR> &availableFormats) {
  for (const auto &availableFormat : availableFormats) {
    if (availableFormat.format == vk::Format::eB8G8R8A8Srgb &&
        availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
      return availableFormat;
    }
  }

  return availableFormats[0];
}

vk::PresentModeKHR SwapChain::chooseSwapPresentMode(
    const std::vector<vk::PresentModeKHR> &availablePresentModes) {
  for (const auto &availablePresentMode : availablePresentModes) {
    if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
      std::cout << "Present mode: Mailbox" << std::endl;
      return availablePresentMode;
    }
  }

  // for (const auto &availablePresentMode : availablePresentModes) {
  //   if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
  //     std::cout << "Present mode: Immediate" << std::endl;
  //     return availablePresentMode;
  //   }
  // }

  std::cout << "Present mode: V-Sync" << std::endl;
  return vk::PresentModeKHR::eFifo;
}

vk::Extent2D SwapChain::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities, vk::Extent2D windowExtent) {
  if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  } else {
    vk::Extent2D actualExtent = windowExtent;
    actualExtent.width = std::max(
        capabilities.minImageExtent.width,
        std::min(capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height = std::max(
        capabilities.minImageExtent.height,
        std::min(capabilities.maxImageExtent.height, actualExtent.height));

    return actualExtent;
  }
}

vk::Format SwapChain::findDepthFormat() {
  std::vector<vk::Format> formats;
  formats.push_back(vk::Format::eD32Sfloat);
  formats.push_back(vk::Format::eD32SfloatS8Uint);
  formats.push_back(vk::Format::eD24UnormS8Uint);
  return findSupportedFormat(
      formats,
      vk::ImageTiling::eOptimal,
      vk::FormatFeatureFlagBits::eDepthStencilAttachment
  );
}

vk::Format SwapChain::findSupportedFormat(
  const std::vector<vk::Format> &candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) {
    for (vk::Format format : candidates) {
      auto props = _physicalDevice.getFormatProperties(format);

      if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
        return format;
      } else if (
          tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
        return format;
      }
    }
    throw std::runtime_error("failed to find supported format!");
  };

}
