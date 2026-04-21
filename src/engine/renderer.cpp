#include "renderer.hpp"

namespace engine {
/*
Renderer::Renderer(const engine::Window& window, vk::Device const& device)
    : _window(window), _device(device) {
    recreateSwapChain();
}

Renderer::~Renderer() {}

vk::CommandBuffer Renderer::beginFrame() {
  assert(!isFrameStarted && "Can't call beginFrame while already in progress");

  auto result = lveSwapChain->acquireNextImage(&currentImageIndex);
  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    recreateSwapChain();
    return nullptr;
  }

  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("failed to acquire swap chain image!");
  }

  isFrameStarted = true;

  auto commandBuffer = getCurrentCommandBuffer();
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
    throw std::runtime_error("failed to begin recording command buffer!");
  }
  return commandBuffer;
}
void Renderer::endFrame() {}
void Renderer::beginSwapChainRenderPass(vk::CommandBuffer commandBuffer) {}
void Renderer::endSwapChainRenderPass(vk::CommandBuffer commandBuffer) {}

void Renderer::createCommandBuffers() {}
void Renderer::freeCommandBuffers() {}

void Renderer::recreateSwapChain() {
  auto extent = _window.getExtent();
  while (extent.width == 0 || extent.height == 0) {
    extent = _window.getExtent();
    _window.waitEvents();
  }

  _device.waitIdle();

  if (_swapChain == nullptr) {
    _swapChain = std::make_unique<engine::SwapChain>(_device, extent);
    return;
  }
  std::shared_ptr<engine::SwapChain> oldSwapChain = std::move(_swapChain);
  _swapChain = std::make_unique<engine::SwapChain>(_device, extent, oldSwapChain);

  //TODO: Implement this check
  //if (!oldSwapChain->compareSwapFormats(*lveSwapChain.get())) {
  //  throw std::runtime_error("Swap chain image(or depth) format has changed!");
  //}
  
}
*/
}  // namespace engine

