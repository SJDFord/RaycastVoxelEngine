#pragma once

#include <memory>
#include <functional>

#include "vulkan/vulkan.hpp"
#include "utils.hpp"

namespace engine {

class SwapChain {
 public:

  SwapChain(
    vk::Device         device,
    vk::PhysicalDevice physicalDevice,
    vk::SurfaceKHR     surface,
    vk::Extent2D       windowExtent,
                      uint32_t                   graphicsFamilyIndex,
                     uint32_t                   presentFamilyIndex,
                     uint32_t                   maxFramesInFlight,
    std::shared_ptr<SwapChain> previous = nullptr
  );

  ~SwapChain();

  SwapChain(const SwapChain &) = delete;
  SwapChain &operator=(const SwapChain &) = delete;

  vk::RenderPass getRenderPass() { return _renderPass; }

  float extentAspectRatio() {
    return static_cast<float>(_extent.width) / static_cast<float>(_extent.height);
  }

  vk::Result acquireNextImage(uint32_t *imageIndex);
  vk::Result submitCommandBuffers(const vk::CommandBuffer *buffers, uint32_t *imageIndex);
  void runRenderPass(const vk::CommandBuffer& commandBuffer, uint32_t frameIndex, uint32_t imageIndex, std::function<void(vk::CommandBuffer const &, int)> renderPassOperation);

  bool compareSwapFormats(const SwapChain &swapChain) const {
    return swapChain._depthFormat == _depthFormat &&
           swapChain._colorFormat == _colorFormat;
  }

 private:

  vk::Format findDepthFormat();

  // Helper functions
vk::SurfaceFormatKHR chooseSwapSurfaceFormat(
    const std::vector<vk::SurfaceFormatKHR> &availableFormats);
  vk::PresentModeKHR chooseSwapPresentMode(
      const std::vector<vk::PresentModeKHR> &availablePresentModes);
  vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities, vk::Extent2D windowExtent);

vk::Format findSupportedFormat(const std::vector<vk::Format> &candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);

  uint32_t _graphicsFamilyIndex;
  uint32_t _presentFamilyIndex;
  uint32_t _maxFramesInFlight;

  vk::Format _colorFormat;
  vk::Format _depthFormat;
  vk::Extent2D _extent;

  std::vector<vk::Framebuffer> _framebuffers;
  vk::RenderPass _renderPass;

  std::vector<vk::Image> _depthImages;
  std::vector<vk::DeviceMemory> _depthImageMemorys;
  std::vector<vk::ImageView> _depthImageViews;
  std::vector<vk::Image> _colorImages;
  std::vector<vk::ImageView> _colorImageViews;

  // TODO: Some private members are only used during creation so can be removed
  vk::Device _device;
  vk::PhysicalDevice _physicalDevice;

  vk::SwapchainKHR _swapChain;
  std::shared_ptr<SwapChain> _oldSwapChain;

  std::vector<vk::Semaphore> _imageAvailableSemaphores;
  std::vector<vk::Semaphore> _renderFinishedSemaphores;
  std::vector<vk::Fence> _inFlightFences;
  std::vector<vk::Fence> _imagesInFlight;
  size_t _currentFrame = 0;
};


}

