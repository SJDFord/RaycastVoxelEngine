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

LveSwapChain::LveSwapChain(LveDevice &deviceRef, vk::Extent2D extent)
    : device{deviceRef}, windowExtent{extent} {
  init();
}

LveSwapChain::LveSwapChain(
    LveDevice &deviceRef, vk::Extent2D extent, std::shared_ptr<LveSwapChain> previous)
    : device{deviceRef}, windowExtent{extent}, oldSwapChain{previous} {
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
    vkDestroyImageView(device.device(), imageView, nullptr);
  }
  swapChainImageViews.clear();

  if (swapChain != nullptr) {
    vkDestroySwapchainKHR(device.device(), swapChain, nullptr);
    swapChain = nullptr;
  }

  for (int i = 0; i < depthImages.size(); i++) {
    vkDestroyImageView(device.device(), depthImageViews[i], nullptr);
    device.destroyImage(depthImages[i]);
    device.freeMemory(depthImageMemorys[i]);
  }

  for (auto framebuffer : swapChainFramebuffers) {
    vkDestroyFramebuffer(device.device(), framebuffer, nullptr);
  }

  vkDestroyRenderPass(device.device(), renderPass, nullptr);

  // cleanup synchronization objects
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroySemaphore(device.device(), renderFinishedSemaphores[i], nullptr);
    vkDestroySemaphore(device.device(), imageAvailableSemaphores[i], nullptr);
    vkDestroyFence(device.device(), inFlightFences[i], nullptr);
  }
}

vk::Result LveSwapChain::acquireNextImage(uint32_t *imageIndex) {
  device.device().waitForFences(inFlightFences[currentFrame], true, std::numeric_limits<uint64_t>::max());
  return device.device().acquireNextImageKHR(swapChain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, imageIndex);
}

vk::Result LveSwapChain::submitCommandBuffers(const vk::CommandBuffer *buffers, uint32_t *imageIndex) {
  if (imagesInFlight[*imageIndex] != VK_NULL_HANDLE) {
    device.device().waitForFences(imagesInFlight[*imageIndex], true, UINT64_MAX);
  }
  imagesInFlight[*imageIndex] = inFlightFences[currentFrame];

  vk::PipelineStageFlags stageFlags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
  vk::SubmitInfo submitInfo = vk::SubmitInfo()
    .setWaitSemaphores(imageAvailableSemaphores[currentFrame])
    .setWaitDstStageMask(stageFlags)
    .setCommandBufferCount(1)
    .setPCommandBuffers(buffers)
    .setSignalSemaphores(renderFinishedSemaphores[currentFrame]);
 
  device.device().resetFences(inFlightFences[currentFrame]);
  device.graphicsQueue().submit(submitInfo, inFlightFences[currentFrame]);
  
  vk::PresentInfoKHR presentInfo = vk::PresentInfoKHR()
    .setWaitSemaphores(renderFinishedSemaphores[currentFrame])
    .setSwapchains(swapChain)
    .setPImageIndices(imageIndex);

  auto result = device.presentQueue().presentKHR(presentInfo);

  currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

  return result;
}

void LveSwapChain::createSwapChain() {
  SwapChainSupportDetails swapChainSupport = device.getSwapChainSupport();

  vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
  vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
  vk::Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities);
  QueueFamilyIndices indices = device.findPhysicalQueueFamilies();

  swapChain = engine::SwapchainBuilder(device.device())
    .setSurfaceCapabilities(swapChainSupport.capabilities)
    .setSurface(device.surface())
    .setSurfaceFormat(surfaceFormat)
    .setExtent(extent)
    .setGraphicsQueueFamilyIndex(indices.graphicsFamily)
    .setPresentQueueFamilyIndex(indices.presentFamily)
    .setPresentMode(presentMode)
    .setOldSwapchain(oldSwapChain == nullptr ? VK_NULL_HANDLE : oldSwapChain->swapChain)
    .build();

  // we only specified a minimum number of images in the swap chain, so the implementation is
  // allowed to create a swap chain with more. That's why we'll first query the final number of
  // images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
  // retrieve the handles.
  
  swapChainImages = device.device().getSwapchainImagesKHR(swapChain);
  swapChainImageFormat = surfaceFormat.format;
  swapChainExtent = extent;
}

void LveSwapChain::createImageViews() {
  swapChainImageViews.resize(swapChainImages.size());
  for (size_t i = 0; i < swapChainImages.size(); i++) {

    swapChainImageViews[i] = device.createImageView(swapChainImages[i], swapChainImageFormat, vk::ImageAspectFlagBits::eColor);
  }
}

void LveSwapChain::createRenderPass() {
  renderPass = engine::RenderPassBuilder(device.device())
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

    swapChainFramebuffers[i] = device.device().createFramebuffer(framebufferInfo);
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

    device.createImageWithInfo(
        imageInfo,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        depthImages[i],
        depthImageMemorys[i]);

    depthImageViews[i] = device.createImageView(depthImages[i], depthFormat, vk::ImageAspectFlagBits::eDepth);
  }
}

void LveSwapChain::createSyncObjects() {
  imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
  imagesInFlight.resize(imageCount(), VK_NULL_HANDLE);

  vk::SemaphoreCreateInfo semaphoreInfo = vk::SemaphoreCreateInfo();
  vk::FenceCreateInfo fenceInfo = vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled);

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    imageAvailableSemaphores[i] = device.device().createSemaphore(semaphoreInfo);
    renderFinishedSemaphores[i] = device.device().createSemaphore(semaphoreInfo);
    inFlightFences[i] = device.device().createFence(fenceInfo);
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

vk::Format LveSwapChain::findDepthFormat() {
  std::vector<vk::Format> formats;
  formats.push_back(vk::Format::eD32Sfloat);
  formats.push_back(vk::Format::eD32SfloatS8Uint);
  formats.push_back(vk::Format::eD24UnormS8Uint);
  return device.findSupportedFormat(
      formats,
      vk::ImageTiling::eOptimal,
      vk::FormatFeatureFlagBits::eDepthStencilAttachment
  );
}

}  // namespace lve
