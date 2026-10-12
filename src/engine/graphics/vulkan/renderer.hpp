#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <print>

#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_raii.hpp"
#include "window.hpp"
#include "swap_chain.hpp"
#include <cassert>

namespace engine {
class Renderer {
 public:
  Renderer(
    engine::Window &window, 
    vk::SurfaceKHR surface,
    vk::Device device,
    vk::PhysicalDevice physicalDevice, 
    vk::CommandPool commandPool,
    uint32_t graphicsFamilyIndex,
    uint32_t presentFamilyIndex,
    uint32_t maxFramesInFlight);
  ~Renderer();

  Renderer(const Renderer &) = delete;
  Renderer &operator=(const Renderer &) = delete;

  vk::RenderPass getSwapChainRenderPass() const { return _swapChain->getRenderPass(); }
  float getAspectRatio() const { return _swapChain->extentAspectRatio(); }
  bool isFrameInProgress() const { return isFrameStarted; }

  vk::CommandBuffer getCurrentCommandBuffer() const {
    assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
    return commandBuffers[currentFrameIndex];
  }

  int getFrameIndex() const {
    assert(isFrameStarted && "Cannot get frame index when frame not in progress");
    return currentFrameIndex;
  }

  vk::CommandBuffer beginFrame();
  void endFrame();
  void beginSwapChainRenderPass(vk::CommandBuffer commandBuffer);
  void endSwapChainRenderPass(vk::CommandBuffer commandBuffer);

 private:
  void createCommandBuffers();
  void freeCommandBuffers();
  void recreateSwapChain();

  engine::Window &_window;
  vk::SurfaceKHR _surface;
  vk::Device _device;
  vk::PhysicalDevice _physicalDevice;
  std::unique_ptr<engine::SwapChain> _swapChain;
  vk::CommandPool _commandPool;
  std::vector<vk::CommandBuffer> commandBuffers;

  uint32_t _graphicsFamilyIndex;
  uint32_t _presentFamilyIndex;
  uint32_t _maxFramesInFlight;
  uint32_t currentImageIndex;
  int currentFrameIndex{0};
  bool isFrameStarted{false};
};

}  // namespace engine