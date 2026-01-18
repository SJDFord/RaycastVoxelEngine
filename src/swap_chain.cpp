#include "swap_chain.hpp"

// std
#include <array>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>

SwapChain::SwapChain(std::shared_ptr<Device> device, vk::Extent2D extent)
    : device{device}, windowExtent{extent} {
  init();
}

SwapChain::SwapChain(
    std::shared_ptr<Device> device, vk::Extent2D extent, std::shared_ptr<SwapChain> previous)
    : device{device}, windowExtent{extent}, oldSwapChain{previous} {
  init();
  oldSwapChain = nullptr;
}

void SwapChain::init() {
  createSwapChain();
  createImageViews();
  createRenderPass();
  createDepthResources();
  createFramebuffers();
  createSyncObjects();
}

SwapChain::~SwapChain() {

}

std::pair<vk::Result, uint32_t> SwapChain::acquireNextImage() {
  device->device().waitForFences(
      **inFlightFences[currentFrame],
      true,
      std::numeric_limits<uint64_t>::max());

  vk::AcquireNextImageInfoKHR acquireNextImageInfo =
      vk::AcquireNextImageInfoKHR()
          .setSwapchain(swapChain)
          .setTimeout(std::numeric_limits<uint64_t>::max())
          .setSemaphore(**imageAvailableSemaphores[currentFrame])
          .setFence(VK_NULL_HANDLE)
          .setDeviceMask(1);
      //.set;
  std::pair<vk::Result, uint32_t> result = device->device().acquireNextImage2KHR(acquireNextImageInfo);

  return result;
}

 vk::Result SwapChain::submitCommandBuffers(
    const vk::raii::CommandBuffer &buffers, uint32_t imageIndex) {
  
  if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
    device->device().waitForFences(
        **imagesInFlight[imageIndex],
        true,
        std::numeric_limits<uint64_t>::max());
  }
  imagesInFlight[imageIndex] = inFlightFences[currentFrame];
  //imagesInFlight.insert(imagesInFlight.begin() + imageIndex, inFlightFences[currentFrame]);
  const vk::Semaphore &signalSemaphore = *(renderFinishedSemaphores[currentFrame]);
  vk::PipelineStageFlags waitFlags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
  const vk::Semaphore &waitSemaphore = *(imageAvailableSemaphores[currentFrame]);
  vk::SubmitInfo submitInfo = vk::SubmitInfo()
    .setWaitSemaphoreCount(1)
    .setWaitSemaphores(waitSemaphore)
    .setWaitDstStageMask(waitFlags)
    .setCommandBufferCount(1)
    .setCommandBuffers(*buffers)
    .setSignalSemaphoreCount(1)
    .setSignalSemaphores(signalSemaphore);

  device->device().resetFences(**inFlightFences[currentFrame]);
  device->graphicsQueue().submit(submitInfo, *inFlightFences[currentFrame]);
  vk::PresentInfoKHR presentInfo =
      vk::PresentInfoKHR()
      .setWaitSemaphoreCount(1)
      .setWaitSemaphores(signalSemaphore)
      .setSwapchainCount(1)
      .setSwapchains(*swapChain)
      .setImageIndices(imageIndex);

  vk::Result result = device->presentQueue().presentKHR(presentInfo);
   
  currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

  return result;
 }

void SwapChain::createSwapChain() {
  SwapChainSupportInfo swapChainSupport = device->getSwapChainSupport();

  vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
  vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
  vk::Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

  uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
  if (swapChainSupport.capabilities.maxImageCount > 0 &&
      imageCount > swapChainSupport.capabilities.maxImageCount) {
    imageCount = swapChainSupport.capabilities.maxImageCount;
  }

  vk::SwapchainCreateInfoKHR createInfo = vk::SwapchainCreateInfoKHR()
          .setSurface(device->surface())
      .setMinImageCount(imageCount)
      .setImageFormat(surfaceFormat.format)
      .setImageColorSpace(surfaceFormat.colorSpace)
      .setImageExtent(extent)
      .setImageArrayLayers(1)
      .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);
  
  uint32_t queueFamilyIndices[] = {
      device->getGraphicsQueueFamily(),
      device->getPresentQueueFamily()};
 
  if (queueFamilyIndices[0] != queueFamilyIndices[1]) {
    createInfo.setImageSharingMode(vk::SharingMode::eConcurrent);
    createInfo.setQueueFamilyIndexCount(2);
    createInfo.setQueueFamilyIndices(queueFamilyIndices);
  } else {
    createInfo.setImageSharingMode(vk::SharingMode::eExclusive);
    createInfo.setQueueFamilyIndexCount(0);
    createInfo.setQueueFamilyIndices({});
  }

  createInfo.setPreTransform(swapChainSupport.capabilities.currentTransform);
  createInfo.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
  createInfo.setPresentMode(presentMode);
  createInfo.setClipped(true);
  createInfo.setOldSwapchain(oldSwapChain == nullptr ? VK_NULL_HANDLE : *oldSwapChain->swapChain);

  swapChain = device->device().createSwapchainKHR(createInfo);

  // we only specified a minimum number of images in the swap chain, so the implementation is
  // allowed to create a swap chain with more. That's why we'll first query the final number of
  // images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
  // retrieve the handles.
  std::vector<vk::Image> images = swapChain.getImages();
  for (auto &image : images) {
    swapChainImages.push_back(std::make_shared<vk::Image>(image));
  }
  /*
  swapChainImages.insert(
      swapChainImages.end(), images.begin(), images.end());
      */

  swapChainImageFormat = surfaceFormat.format;
  swapChainExtent = extent;
}

void SwapChain::createImageViews() {
    // TODO
  swapChainImageViews.resize(swapChainImages.size());
  for (size_t i = 0; i < swapChainImages.size(); i++) {
    vk::ImageSubresourceRange subresourceRange = vk::ImageSubresourceRange()
        .setAspectMask(vk::ImageAspectFlagBits::eColor)
        .setBaseMipLevel(0)
        .setLevelCount(1)
        .setBaseArrayLayer(0)
        .setLayerCount(1);
    vk::ImageViewCreateInfo viewInfo = vk::ImageViewCreateInfo()
                                           .setImage(*swapChainImages[i])
                                           .setViewType(vk::ImageViewType::e2D)
                                           .setFormat(swapChainImageFormat)
                                           .setSubresourceRange(subresourceRange);

    swapChainImageViews[i] = std::make_shared<vk::raii::ImageView>(device->device().createImageView(viewInfo));
  }
}

void SwapChain::createRenderPass() {
  vk::AttachmentDescription depthAttachment =
      vk::AttachmentDescription()
          .setFormat(findDepthFormat())
          .setSamples(vk::SampleCountFlagBits::e1)
          .setLoadOp(vk::AttachmentLoadOp::eClear)
          .setStoreOp(vk::AttachmentStoreOp::eDontCare)
          .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
          .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
          .setInitialLayout(vk::ImageLayout::eUndefined)
          .setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
  vk::AttachmentReference depthAttachmentRef = vk::AttachmentReference()
      .setAttachment(1)
      .setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

  vk::AttachmentDescription colorAttachment =
      vk::AttachmentDescription()
          .setFormat(getSwapChainImageFormat())
          .setSamples(vk::SampleCountFlagBits::e1)
          .setLoadOp(vk::AttachmentLoadOp::eClear)
          .setStoreOp(vk::AttachmentStoreOp::eDontCare)
          .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
          .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
          .setInitialLayout(vk::ImageLayout::eUndefined)
          .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

    vk::AttachmentReference colorAttachmentRef =
        vk::AttachmentReference().setAttachment(0).setLayout(
            vk::ImageLayout::eColorAttachmentOptimal);

  vk::SubpassDescription subpass = vk::SubpassDescription()
                                         .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
                                         .setColorAttachmentCount(1)
                                         .setColorAttachments(colorAttachmentRef)
                                         .setPDepthStencilAttachment(&depthAttachmentRef);        
 
  vk::SubpassDependency dependency = vk::SubpassDependency()
                                         .setDstSubpass(0)
                                         .setDstAccessMask(
                                             vk::AccessFlagBits::eColorAttachmentWrite |
                                             vk::AccessFlagBits::eDepthStencilAttachmentWrite)
                                         .setDstStageMask(
                                             vk::PipelineStageFlagBits::eColorAttachmentOutput |
                                             vk::PipelineStageFlagBits::eEarlyFragmentTests)
                                         .setSrcSubpass(VK_SUBPASS_EXTERNAL)

                                         .setSrcStageMask(
                                             vk::PipelineStageFlagBits::eColorAttachmentOutput |
                                             vk::PipelineStageFlagBits::eEarlyFragmentTests);
  
  std::array<vk::AttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
  vk::RenderPassCreateInfo renderPassInfo =
      vk::RenderPassCreateInfo()
          .setAttachmentCount(static_cast<uint32_t>(attachments.size()))
          .setAttachments(attachments)
          .setSubpassCount(1)
          .setSubpasses(subpass)
          .setDependencyCount(1)
          .setDependencies(dependency);

  renderPass = device->device().createRenderPass(renderPassInfo);
}

void SwapChain::createFramebuffers() {
  swapChainFramebuffers.resize(imageCount());
  for (size_t i = 0; i < imageCount(); i++) {
    std::array<const vk::ImageView, 2> attachments = {*(swapChainImageViews[i]), *(depthImageViews[i])};

    vk::Extent2D swapChainExtent = getSwapChainExtent();
    vk::FramebufferCreateInfo framebufferInfo = vk::FramebufferCreateInfo()
        .setRenderPass(renderPass)
        .setAttachmentCount(static_cast<uint32_t>(attachments.size()))
        .setAttachments(attachments)
        .setWidth(swapChainExtent.width)
        .setHeight(swapChainExtent.height)
        .setLayers(1);

    swapChainFramebuffers[i] = std::make_shared<vk::raii::Framebuffer>(device->device().createFramebuffer(framebufferInfo));
  }
}

void SwapChain::createDepthResources() {
  vk::Format depthFormat = findDepthFormat();
  swapChainDepthFormat = depthFormat;
  vk::Extent2D swapChainExtent = getSwapChainExtent();

  depthImages.resize(imageCount());
  depthImageMemorys.resize(imageCount());
  depthImageViews.resize(imageCount());

  for (int i = 0; i < depthImages.size(); i++) {
    vk::Extent3D extent = vk::Extent3D()
                              .setWidth(swapChainExtent.width)
                              .setHeight(swapChainExtent.height)
                              .setDepth(1);
    vk::ImageCreateInfo imageInfo = vk::ImageCreateInfo()
                                        .setImageType(vk::ImageType::e2D)
                                        .setExtent(extent)
                                        .setMipLevels(1)
                                        .setArrayLayers(1)
                                        .setFormat(depthFormat)
                                        .setTiling(vk::ImageTiling::eOptimal)
                                        .setInitialLayout(vk::ImageLayout::eUndefined)
                                        .setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment)
                                        .setSamples(vk::SampleCountFlagBits::e1)
                                        .setSharingMode(vk::SharingMode::eExclusive);
    // TODO: .setFlags(vk::ImageCreateFlagBits::) imageInfo.flags = 0;

    auto result = device->createImageWithInfo(imageInfo, vk::MemoryPropertyFlagBits::eDeviceLocal);
    result.image.swap(depthImages[i]);
    result.imageMemory.swap(depthImageMemorys[i]);

    vk::ImageSubresourceRange subresourceRange = vk::ImageSubresourceRange()
                                                     .setAspectMask(vk::ImageAspectFlagBits::eDepth)
                                                     .setBaseMipLevel(0)
                                                     .setLevelCount(1)
                                                     .setBaseArrayLayer(0)
                                                     .setLayerCount(1);

    vk::ImageViewCreateInfo viewInfo = vk::ImageViewCreateInfo()
        .setImage(*depthImages[i])
        .setViewType(vk::ImageViewType::e2D)
        .setFormat(depthFormat)
        .setSubresourceRange(subresourceRange);
    depthImageViews[i] = std::make_shared < vk::raii::ImageView>(device->device().createImageView(viewInfo));
  }
}

void SwapChain::createSyncObjects() {
  imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE);
  renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE);
  inFlightFences.resize(MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE);
  imagesInFlight.resize(imageCount(), VK_NULL_HANDLE);

  vk::SemaphoreCreateInfo semaphoreInfo = vk::SemaphoreCreateInfo();

  vk::FenceCreateInfo fenceInfo = vk::FenceCreateInfo()
      .setFlags(vk::FenceCreateFlagBits::eSignaled);

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    imageAvailableSemaphores[i] = std::make_shared<vk::raii::Semaphore>(device->device().createSemaphore(semaphoreInfo));
    renderFinishedSemaphores[i] = std::make_shared<vk::raii::Semaphore>(device->device().createSemaphore(semaphoreInfo));
    inFlightFences[i] = std::make_shared<vk::raii::Fence>(device->device().createFence(fenceInfo));
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
  vk::PresentModeKHR::eFifo;
}

vk::Extent2D SwapChain::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities) {
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
  return device->findSupportedFormat(
      {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
      vk::ImageTiling::eOptimal,
      vk::FormatFeatureFlagBits::eDepthStencilAttachment
  );
}