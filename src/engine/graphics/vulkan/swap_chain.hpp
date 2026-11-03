#pragma once

#include <memory>
#include <functional>

#include "vulkan/vulkan.hpp"
#include "utils.hpp"

namespace engine {

class SwapChain {
 public:

  SwapChain(
    vk::Instance instance,
    vk::Device         device,
    vk::PhysicalDevice physicalDevice,
    vk::SurfaceKHR     surface,
    vk::Extent2D       windowExtent,
    uint32_t                   graphicsFamilyIndex,
    uint32_t                   presentFamilyIndex,
    uint32_t                   maxFramesInFlight,
    bool enableDynamicRendering,
    std::shared_ptr<SwapChain> previous = nullptr
  );

  ~SwapChain();

  SwapChain(const SwapChain &) = delete;
  SwapChain &operator=(const SwapChain &) = delete;

  vk::RenderPass getRenderPass() { return _renderPass; }
  vk::Format getColorFormat() { return _colorFormat; }
  vk::Format getDepthFormat() { return _depthFormat; }

  float extentAspectRatio() {
    return static_cast<float>(_extent.width) / static_cast<float>(_extent.height);
  }

  vk::Result acquireNextImage(uint32_t *imageIndex);
  vk::Result submitCommandBuffers(const vk::CommandBuffer *buffers, uint32_t *imageIndex);
  void runRenderPass(const vk::CommandBuffer& commandBuffer, uint32_t frameIndex, uint32_t imageIndex, std::function<void(vk::CommandBuffer const &, int)> renderPassOperation);
  void run(const vk::CommandBuffer& commandBuffer, uint32_t frameIndex, uint32_t imageIndex, std::function<void(vk::CommandBuffer const &, int)> renderOperation);


  bool compareSwapFormats(const SwapChain &swapChain) const {
    return swapChain._depthFormat == _depthFormat &&
           swapChain._colorFormat == _colorFormat;
  }

 private:

  bool _enableDynamicRendering;

  vk::Format findDepthFormat();

  // Helper functions
  vk::SurfaceFormatKHR chooseSwapSurfaceFormat(
    const std::vector<vk::SurfaceFormatKHR> &availableFormats);
  vk::PresentModeKHR chooseSwapPresentMode(
      const std::vector<vk::PresentModeKHR> &availablePresentModes);
  vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities, vk::Extent2D windowExtent);

  vk::Format findSupportedFormat(const std::vector<vk::Format> &candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);

  void transitionImageLayout(
    vk::CommandBuffer                commandBuffer,
    vk::Image                        image,
    vk::PipelineStageFlags           srcStageMask,
    vk::PipelineStageFlags           dstStageMask,
    vk::AccessFlags                  srcAccessMask,
    vk::AccessFlags                  dstAccessMask,
    vk::ImageLayout                  oldLayout,
    vk::ImageLayout                  newLayout,
    vk::ImageSubresourceRange const &subresourceRange
  );

  void transitionImageLayout(
    vk::CommandBuffer                commandBuffer,
    vk::Image                        image,
    vk::ImageLayout                  oldLayout,
    vk::ImageLayout                  newLayout,
    vk::ImageSubresourceRange const &subresourceRange
  );


  vk::PipelineStageFlags getPipelineStageFlags(vk::ImageLayout layout);
  vk::AccessFlags getAccessFlags(vk::ImageLayout layout);
  

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

  vk::Instance _instance;
	PFN_vkCmdBeginRenderingKHR vkCmdBeginRenderingKHR{VK_NULL_HANDLE};
	PFN_vkCmdEndRenderingKHR   vkCmdEndRenderingKHR{VK_NULL_HANDLE};
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

