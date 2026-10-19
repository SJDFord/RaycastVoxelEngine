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
  bool renderFrame(
    std::function<void(vk::CommandBuffer const &, int)> preRender, 
    std::initializer_list<std::function<void(vk::CommandBuffer const &, int)>> renderPassOperations,
    std::function<void(vk::CommandBuffer const &, int)> postRender
  );

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
  std::vector<vk::CommandBuffer> _commandBuffers;

  uint32_t _graphicsFamilyIndex;
  uint32_t _presentFamilyIndex;
  uint32_t _maxFramesInFlight;
  uint32_t _currentImageIndex;
  int _currentFrameIndex{0};
};

}  // namespace engine