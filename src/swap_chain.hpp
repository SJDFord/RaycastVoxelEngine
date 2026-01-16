#pragma once

#include "device.hpp"

// std lib headers
#include <memory>
#include <string>
#include <vector>

class SwapChain {
 public:
  static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

  SwapChain(std::shared_ptr<Device> device, vk::Extent2D windowExtent);
  SwapChain(
      std::shared_ptr<Device> device,
      vk::Extent2D windowExtent,
      std::shared_ptr<SwapChain> previous);

  ~SwapChain();

  SwapChain(const SwapChain &) = delete;
  SwapChain &operator=(const SwapChain &) = delete;

  vk::raii::Framebuffer& getFrameBuffer(int index) { return *(swapChainFramebuffers[index]); }
  vk::raii::RenderPass& getRenderPass() { return renderPass; }
  vk::raii::ImageView& getImageView(int index) { return *(swapChainImageViews[index]); }
  size_t imageCount() { return swapChainImages.size(); }
  vk::Format getSwapChainImageFormat() { return swapChainImageFormat; }
  vk::Extent2D getSwapChainExtent() { return swapChainExtent; }
  uint32_t width() { return swapChainExtent.width; }
  uint32_t height() { return swapChainExtent.height; }

  float extentAspectRatio() {
    return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);
  }
  vk::Format findDepthFormat();

  std::pair<vk::Result, uint32_t> acquireNextImage();
  vk::Result submitCommandBuffers(const vk::raii::CommandBuffer &buffers, uint32_t imageIndex);

  bool compareSwapFormats(const SwapChain &swapChain) const {
    return swapChain.swapChainDepthFormat == swapChainDepthFormat &&
           swapChain.swapChainImageFormat == swapChainImageFormat;
  }

 private:
  void init();
  void createSwapChain();
  void createImageViews();
  void createDepthResources();
  void createRenderPass();
  void createFramebuffers();
  void createSyncObjects();

  // Helper functions
  vk::SurfaceFormatKHR chooseSwapSurfaceFormat(
      const std::vector<vk::SurfaceFormatKHR> &availableFormats);
  vk::PresentModeKHR chooseSwapPresentMode(
      const std::vector<vk::PresentModeKHR> &availablePresentModes);
  vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities);

  vk::Format swapChainImageFormat;
  vk::Format swapChainDepthFormat;
  vk::Extent2D swapChainExtent;

  std::vector<vk::raii::Framebuffer*> swapChainFramebuffers;
  vk::raii::RenderPass renderPass = VK_NULL_HANDLE;

  std::vector<std::shared_ptr<vk::raii::Image>> depthImages;
  std::vector<std::shared_ptr<vk::raii::DeviceMemory>> depthImageMemorys;
  std::vector<std::shared_ptr<vk::raii::ImageView>> depthImageViews;
  std::vector<std::shared_ptr<vk::Image>> swapChainImages;
  std::vector<std::shared_ptr<vk::raii::ImageView>> swapChainImageViews;

  std::shared_ptr<Device> device;
  vk::Extent2D windowExtent;

  vk::raii::SwapchainKHR swapChain = VK_NULL_HANDLE;
  std::shared_ptr<SwapChain> oldSwapChain;

  std::vector<vk::raii::Semaphore*> imageAvailableSemaphores;
  std::vector<vk::raii::Semaphore*> renderFinishedSemaphores;
  std::vector<vk::raii::Fence*> inFlightFences;
  std::vector<vk::raii::Fence*> imagesInFlight;
  size_t currentFrame = 0;
};