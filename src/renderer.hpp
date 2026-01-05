#pragma once

#include "device.hpp"
#include "swap_chain.hpp"
#include "window.hpp"


// std
#include <cassert>
#include <memory>
#include <vector>
/*
class Renderer {
 public:
  Renderer(Window &window, Device &device);
  ~Renderer();

  Renderer(const Renderer &) = delete;
  Renderer &operator=(const Renderer &) = delete;

  vk::raii::RenderPass& getSwapChainRenderPass() const { return swapChain->getRenderPass(); }
  float getAspectRatio() const { return swapChain->extentAspectRatio(); }
  bool isFrameInProgress() const { return isFrameStarted; }

  const vk::raii::CommandBuffer& getCurrentCommandBuffer() const {
    assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
    return commandBuffers[currentFrameIndex];
  }

  int getFrameIndex() const {
    assert(isFrameStarted && "Cannot get frame index when frame not in progress");
    return currentFrameIndex;
  }

  const vk::raii::CommandBuffer& beginFrame();
  void endFrame();
  void beginSwapChainRenderPass(const vk::raii::CommandBuffer& commandBuffer);
  void endSwapChainRenderPass(const vk::raii::CommandBuffer& commandBuffer);

 private:
  void createCommandBuffers();
  void freeCommandBuffers();
  void recreateSwapChain();

  Window &window;
  Device &device;
  std::unique_ptr<SwapChain> swapChain;
  std::vector<vk::raii::CommandBuffer> commandBuffers;

  uint32_t currentImageIndex;
  int currentFrameIndex{0};
  bool isFrameStarted{false};
};
*/