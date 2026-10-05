#include "lve_swap_chain.hpp"

#include "./engine/swapchain_builder.hpp"
#include "./engine/render_pass_builder.hpp"


// std
#include <array>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>

namespace lve {

LveSwapChain::LveSwapChain(
    vk::Device         device,
    vk::PhysicalDevice physicalDevice,
    vk::SurfaceKHR     surface,
    vk::Extent2D       extent,
uint32_t                   graphicsFamilyIndex,
                     uint32_t                   presentFamilyIndex,
                     uint32_t                   maxFramesInFlight
)
    : _device{device}, _physicalDevice{physicalDevice}, _windowExtent{extent}, _surface{surface}, _graphicsFamilyIndex{graphicsFamilyIndex}, _presentFamilyIndex{presentFamilyIndex}, _maxFramesInFlight(maxFramesInFlight) {
  init();
}

LveSwapChain::LveSwapChain(
    vk::Device         device,
    vk::PhysicalDevice physicalDevice,
    vk::SurfaceKHR     surface,
    vk::Extent2D       extent,
uint32_t                   graphicsFamilyIndex,
                     uint32_t                   presentFamilyIndex,
                     uint32_t                   maxFramesInFlight,
    std::shared_ptr<LveSwapChain> previous)
    : _device{device}, _physicalDevice{physicalDevice}, _windowExtent{extent}, _surface{surface}, _graphicsFamilyIndex{graphicsFamilyIndex}, _presentFamilyIndex{presentFamilyIndex}, _maxFramesInFlight(maxFramesInFlight), oldSwapChain{previous} {
  init();
  oldSwapChain = nullptr;
}

void LveSwapChain::init() {
  createSwapChain();
  createImageViews();
  createRenderPass();
  createDepthResources();
  createFramebuffers();
  createSyncObjects();
}

LveSwapChain::~LveSwapChain() {
  for (auto imageView : swapChainImageViews) {
    _device.destroyImageView(imageView); 
  }
  swapChainImageViews.clear();

  if (swapChain != nullptr) {
    _device.destroySwapchainKHR(swapChain);
    swapChain = nullptr;
  }

  for (int i = 0; i < depthImages.size(); i++) {
    _device.destroyImageView(depthImageViews[i]);
    _device.destroyImage(depthImages[i]);
    _device.freeMemory(depthImageMemorys[i]);
  }

  for (auto framebuffer : swapChainFramebuffers) {
    _device.destroyFramebuffer(framebuffer);
  }

  _device.destroyRenderPass(renderPass);

  // cleanup synchronization objects
  for (size_t i = 0; i < _maxFramesInFlight; i++) {
    _device.destroySemaphore(renderFinishedSemaphores[i]);
    _device.destroySemaphore(imageAvailableSemaphores[i]);
    _device.destroyFence(inFlightFences[i]);
  }
}

vk::Result LveSwapChain::acquireNextImage(uint32_t *imageIndex) {
  _device.waitForFences(inFlightFences[currentFrame], true, std::numeric_limits<uint64_t>::max());
  return _device.acquireNextImageKHR(swapChain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, imageIndex);
}

vk::Result LveSwapChain::submitCommandBuffers(const vk::CommandBuffer *buffers, uint32_t *imageIndex) {
  if (imagesInFlight[*imageIndex] != VK_NULL_HANDLE) {
    _device.waitForFences(imagesInFlight[*imageIndex], true, UINT64_MAX);
  }
  imagesInFlight[*imageIndex] = inFlightFences[currentFrame];

  vk::PipelineStageFlags stageFlags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
  vk::SubmitInfo submitInfo = vk::SubmitInfo()
    .setWaitSemaphores(imageAvailableSemaphores[currentFrame])
    .setWaitDstStageMask(stageFlags)
    .setCommandBufferCount(1)
    .setPCommandBuffers(buffers)
    .setSignalSemaphores(renderFinishedSemaphores[currentFrame]);
 
  _device.resetFences(inFlightFences[currentFrame]);
  _device.getQueue(_graphicsFamilyIndex, 0).submit(submitInfo, inFlightFences[currentFrame]);
  
  vk::PresentInfoKHR presentInfo = vk::PresentInfoKHR()
    .setWaitSemaphores(renderFinishedSemaphores[currentFrame])
    .setSwapchains(swapChain)
    .setPImageIndices(imageIndex);

  auto result = _device.getQueue(_presentFamilyIndex, 0).presentKHR(presentInfo);

  currentFrame = (currentFrame + 1) % _maxFramesInFlight;

  return result;
}

void LveSwapChain::createSwapChain() {
  auto capabilities = _physicalDevice.getSurfaceCapabilitiesKHR(_surface);
  auto formats = _physicalDevice.getSurfaceFormatsKHR(_surface);
  auto presentModes = _physicalDevice.getSurfacePresentModesKHR(_surface);
  vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(formats);
  vk::PresentModeKHR presentMode = chooseSwapPresentMode(presentModes);
  vk::Extent2D extent = chooseSwapExtent(capabilities);
  //QueueFamilyIndices indices = device.findPhysicalQueueFamilies();

  swapChain = engine::SwapchainBuilder(_device)
    .setSurfaceCapabilities(capabilities)
    .setSurface(_surface)
    .setSurfaceFormat(surfaceFormat)
    .setExtent(extent)
    .setGraphicsQueueFamilyIndex(_graphicsFamilyIndex)
    .setPresentQueueFamilyIndex(_presentFamilyIndex)
    .setPresentMode(presentMode)
    .setOldSwapchain(oldSwapChain == nullptr ? VK_NULL_HANDLE : oldSwapChain->swapChain)
    .build();

  // we only specified a minimum number of images in the swap chain, so the implementation is
  // allowed to create a swap chain with more. That's why we'll first query the final number of
  // images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
  // retrieve the handles.
  
  swapChainImages = _device.getSwapchainImagesKHR(swapChain);
  swapChainImageFormat = surfaceFormat.format;
  swapChainExtent = extent;
}

void LveSwapChain::createImageViews() {
  swapChainImageViews.resize(swapChainImages.size());
  for (size_t i = 0; i < swapChainImages.size(); i++) {
      vk::ImageSubresourceRange subresourceRange = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor)
        .setBaseMipLevel(0)
        .setLevelCount(1)
        .setBaseArrayLayer(0)
        .setLayerCount(1);

      vk::ImageViewCreateInfo viewInfo = vk::ImageViewCreateInfo()
        .setImage(swapChainImages[i])
        .setViewType(vk::ImageViewType::e2D)
        .setFormat(swapChainImageFormat)
        .setSubresourceRange(subresourceRange);

      swapChainImageViews[i] = _device.createImageView(viewInfo);
  }
}

void LveSwapChain::createRenderPass() {
  renderPass = engine::RenderPassBuilder(_device)
    .setColorFormat(getSwapChainImageFormat())
    .setDepthFormat(findDepthFormat())
    .build();
}

void LveSwapChain::createFramebuffers() {
  swapChainFramebuffers.resize(imageCount());
  for (size_t i = 0; i < imageCount(); i++) {
    std::array<vk::ImageView, 2> attachments = {swapChainImageViews[i], depthImageViews[i]};

    vk::Extent2D swapChainExtent = getSwapChainExtent();
    vk::FramebufferCreateInfo framebufferInfo = vk::FramebufferCreateInfo()
      .setRenderPass(renderPass)
      .setAttachments(attachments)
      .setWidth(swapChainExtent.width)
      .setHeight(swapChainExtent.height)
      .setLayers(1);

    swapChainFramebuffers[i] = _device.createFramebuffer(framebufferInfo);
  }
}

void LveSwapChain::createDepthResources() {
  vk::Format depthFormat = findDepthFormat();
  swapChainDepthFormat = depthFormat;
  VkExtent2D swapChainExtent = getSwapChainExtent();

  depthImages.resize(imageCount());
  depthImageMemorys.resize(imageCount());
  depthImageViews.resize(imageCount());

  for (int i = 0; i < depthImages.size(); i++) {
    vk::ImageCreateInfo imageInfo = vk::ImageCreateInfo()
      .setImageType(vk::ImageType::e2D)
      .setExtent(vk::Extent3D(swapChainExtent, 1))
      .setMipLevels(1)
      .setArrayLayers(1)
      .setFormat(depthFormat)
      .setTiling(vk::ImageTiling::eOptimal)
      .setInitialLayout(vk::ImageLayout::eUndefined)
      .setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment)
      .setSamples(vk::SampleCountFlagBits::e1)
      .setSharingMode(vk::SharingMode::eExclusive);

    depthImages[i] = _device.createImage(imageInfo);
    vk::MemoryRequirements memRequirements = _device.getImageMemoryRequirements(depthImages[i]);

    vk::MemoryAllocateInfo allocInfo = vk::MemoryAllocateInfo()
      .setAllocationSize(memRequirements.size)
      .setMemoryTypeIndex(findMemoryType(memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal));

    depthImageMemorys[i] = _device.allocateMemory(allocInfo);

    _device.bindImageMemory(depthImages[i], depthImageMemorys[i], 0);

      vk::ImageSubresourceRange subresourceRange = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth)
      .setBaseMipLevel(0)
      .setLevelCount(1)
      .setBaseArrayLayer(0)
      .setLayerCount(1);

    vk::ImageViewCreateInfo viewInfo = vk::ImageViewCreateInfo()
      .setImage(depthImages[i])
      .setViewType(vk::ImageViewType::e2D)
      .setFormat(depthFormat)
      .setSubresourceRange(subresourceRange);

    depthImageViews[i] = _device.createImageView(viewInfo);
  }
}

void LveSwapChain::createSyncObjects() {
  imageAvailableSemaphores.resize(_maxFramesInFlight);
  renderFinishedSemaphores.resize(_maxFramesInFlight);
  inFlightFences.resize(_maxFramesInFlight);
  imagesInFlight.resize(imageCount(), VK_NULL_HANDLE);

  vk::SemaphoreCreateInfo semaphoreInfo = vk::SemaphoreCreateInfo();
  vk::FenceCreateInfo fenceInfo = vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled);

  for (size_t i = 0; i < _maxFramesInFlight; i++) {
    imageAvailableSemaphores[i] = _device.createSemaphore(semaphoreInfo);
    renderFinishedSemaphores[i] = _device.createSemaphore(semaphoreInfo);
    inFlightFences[i] = _device.createFence(fenceInfo);
  }
}

vk::SurfaceFormatKHR LveSwapChain::chooseSwapSurfaceFormat(
    const std::vector<vk::SurfaceFormatKHR> &availableFormats) {
  for (const auto &availableFormat : availableFormats) {
    if (availableFormat.format == vk::Format::eB8G8R8A8Srgb &&
        availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
      return availableFormat;
    }
  }

  return availableFormats[0];
}

vk::PresentModeKHR LveSwapChain::chooseSwapPresentMode(
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

vk::Extent2D LveSwapChain::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities) {
  if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  } else {
    vk::Extent2D actualExtent = _windowExtent;
    actualExtent.width = std::max(
        capabilities.minImageExtent.width,
        std::min(capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height = std::max(
        capabilities.minImageExtent.height,
        std::min(capabilities.maxImageExtent.height, actualExtent.height));

    return actualExtent;
  }
}

vk::Format LveSwapChain::findDepthFormat() {
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

uint32_t LveSwapChain::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {

  auto memoryProperties = _physicalDevice.getMemoryProperties();
  for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) &&
        (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }

  throw std::runtime_error("failed to find suitable memory type!");
}

  vk::Format LveSwapChain::findSupportedFormat(
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

}  // namespace lve
