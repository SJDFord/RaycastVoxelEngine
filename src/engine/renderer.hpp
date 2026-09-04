#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_raii.hpp"
#include "window.hpp"
#include "swap_chain.hpp"
#include <cassert>

namespace engine {

class Renderer {
 public:
  Renderer(
    engine::Window& window, 
    vk::Device const& device,
    vk::PhysicalDevice const & physicalDevice,
    vk::ImageUsageFlags        usage,
    uint32_t                   graphicsFamilyIndex,
    uint32_t                   presentFamilyIndex,
    uint32_t                   maxFramesInFlight);
  ~Renderer();

  Renderer(const Renderer&) = delete;
  Renderer& operator=(const Renderer&) = delete;

  vk::CommandBuffer beginFrame(/*bool &hasFrame*/);
  void endFrame();

  vk::CommandBuffer getCurrentCommandBuffer() const {
    assert(_isFrameStarted && "Cannot get command buffer when frame not in progress");
    return _commandBuffers[_currentFrameIndex];
  }

  int getFrameIndex() const {
    assert(_isFrameStarted && "Cannot get frame index when frame not in progress");
    return _currentFrameIndex;
  }

  float getAspectRatio() {
    auto extent = _window.getExtent();
    return static_cast<float>(extent.width) / static_cast<float>(extent.height);
  }
 
private:
  engine::Window& _window;
  const vk::Device& _device;
  const vk::PhysicalDevice& _physicalDevice;
  const vk::ImageUsageFlags _imageUsageFlags;
  const uint32_t _graphicsFamilyIndex;
  const uint32_t _presentFamilyIndex;
  const uint32_t _maxFramesInFlight;

  void createCommandBuffers();
  void freeCommandBuffers();
  void recreateSwapChain();

  vk::CommandPool _commandPool;
  std::unique_ptr<engine::SwapChain> _swapChain;
  std::vector<vk::CommandBuffer> _commandBuffers;

  uint32_t _currentImageIndex;
  int _currentFrameIndex{0};
  bool _isFrameStarted{false};
};

}  // namespace engine