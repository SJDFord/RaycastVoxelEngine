#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_raii.hpp"
#include "window.hpp"
#include "swap_chain.hpp"

namespace engine {
/*
class Renderer {
 public:
  Renderer(const engine::Window& window, vk::Device const& device);
  ~Renderer();

  Renderer(const Renderer&) = delete;
  Renderer& operator=(const Renderer&) = delete;

  vk::CommandBuffer beginFrame();
  void endFrame();
  void beginSwapChainRenderPass(vk::CommandBuffer commandBuffer);
  void endSwapChainRenderPass(vk::CommandBuffer commandBuffer);


 private:
  const engine::Window& _window;
  const vk::Device& _device;

  void createCommandBuffers();
  void freeCommandBuffers();
  void recreateSwapChain();


  std::unique_ptr<engine::SwapChain> _swapChain;

  uint32_t _currentImageIndex;
  int _currentFrameIndex{0};
  bool _isFrameStarted{false};
};
*/
}  // namespace engine