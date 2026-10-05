#pragma once

#include "lve_device.hpp"
#include "lve_swap_chain.hpp"
#include "./engine/window.hpp"

// std
#include <cassert>
#include <memory>
#include <vector>

namespace lve {
class LveRenderer {
 public:
  LveRenderer(engine::Window &window, LveDevice &device, uint32_t maxFramesInFlight);
  ~LveRenderer();

  LveRenderer(const LveRenderer &) = delete;
  LveRenderer &operator=(const LveRenderer &) = delete;

  vk::RenderPass getSwapChainRenderPass() const { return lveSwapChain->getRenderPass(); }
  float getAspectRatio() const { return lveSwapChain->extentAspectRatio(); }
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
  LveDevice &lveDevice;
  std::unique_ptr<LveSwapChain> lveSwapChain;
  std::vector<vk::CommandBuffer> commandBuffers;

  uint32_t _maxFramesInFlight;
  uint32_t currentImageIndex;
  int currentFrameIndex{0};
  bool isFrameStarted{false};
};
}  // namespace lve
