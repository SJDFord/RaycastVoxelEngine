#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <print>

#include "vulkan/vulkan.hpp"
#include "window.hpp"
#include "swap_chain.hpp"
#include <cassert>
#include <functional>

namespace engine {
class Renderer {
 public:
  Renderer(
    vk::Instance instance,
    engine::Window &window, 
    vk::SurfaceKHR surface,
    vk::Device device,
    vk::PhysicalDevice physicalDevice, 
    vk::CommandPool commandPool,
    uint32_t graphicsFamilyIndex,
    uint32_t presentFamilyIndex,
    uint32_t maxFramesInFlight,
    bool enableDynamicRendering);
  ~Renderer();

  Renderer(const Renderer &) = delete;
  Renderer &operator=(const Renderer &) = delete;

  vk::RenderPass getSwapChainRenderPass() const { return _swapChain->getRenderPass(); }
  vk::Format getSwapChainColorFormat() { return _swapChain->getColorFormat(); }
  vk::Format getSwapChainDepthFormat() { return _swapChain->getDepthFormat(); }

  float getAspectRatio() const { return _swapChain->extentAspectRatio(); }
  bool renderFrame(
    std::function<void(vk::CommandBuffer const &, int)> preRender, 
    std::initializer_list<std::function<void(vk::CommandBuffer const &, int)>> renderOperations,
    std::function<void(vk::CommandBuffer const &, int)> postRender
  );

 private:
  vk::Instance _instance;
  bool _enableDynamicRendering;

  void createCommandBuffers();
  void freeCommandBuffers();
  void recreateSwapChain(vk::Extent2D extent);
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
  engine::Window &_window;
  vk::SurfaceKHR _surface;
  vk::Device _device;
  vk::PhysicalDevice _physicalDevice;
  std::unique_ptr<engine::SwapChain> _swapChain;
  vk::CommandPool _commandPool;
  std::vector<vk::CommandBuffer> _commandBuffers;

  uint32_t _graphicsFamilyIndex;
  uint32_t _presentFamilyIndex;
  uint32_t _maxFramesInFlight;
  uint32_t _currentImageIndex;
  int _currentFrameIndex{0};
};

}  // namespace engine