#include "renderer.hpp"
#include <print>

namespace engine {

Renderer::Renderer(engine::Window& window,    
    vk::SurfaceKHR surface,  vk::Device device,
    vk::PhysicalDevice physicalDevice, vk::CommandPool commandPool, 
    uint32_t graphicsFamilyIndex,
    uint32_t presentFamilyIndex,
    uint32_t maxFramesInFlight)
    : 
    _window{window},
    _surface{surface}, 
    _device{device}, 
    _physicalDevice{physicalDevice}, 
    _commandPool{commandPool}, 
    _graphicsFamilyIndex{graphicsFamilyIndex},
    _presentFamilyIndex{presentFamilyIndex},
    _maxFramesInFlight{maxFramesInFlight} {
  recreateSwapChain();
  createCommandBuffers();
}

Renderer::~Renderer() { freeCommandBuffers(); }

void Renderer::recreateSwapChain() {
  auto extent = _window.getExtent();
  while (extent.width == 0 || extent.height == 0) {
    extent = _window.getExtent();
    _window.waitEvents();
  }

  _device.waitIdle();
  
  if (_swapChain == nullptr) {
    _swapChain = std::make_unique<engine::SwapChain>(
      _device,
      _physicalDevice,
      _surface, 
      _window.getExtent(),
      _graphicsFamilyIndex,
      _presentFamilyIndex, 
      _maxFramesInFlight 
    );
  } else {
    std::shared_ptr<engine::SwapChain> oldSwapChain = std::move(_swapChain);
    _swapChain = std::make_unique<engine::SwapChain>(
      _device,
      _physicalDevice,
      _surface,
      _window.getExtent(),
      _graphicsFamilyIndex,
      _presentFamilyIndex, 
      _maxFramesInFlight,
      oldSwapChain
    );

    if (!oldSwapChain->compareSwapFormats(*_swapChain.get())) {
      throw std::runtime_error("Swap chain image(or depth) format has changed!");
    }
  }
}

void Renderer::createCommandBuffers() {
  commandBuffers.resize(_maxFramesInFlight);

  vk::CommandBufferAllocateInfo allocInfo = vk::CommandBufferAllocateInfo()
    .setLevel(vk::CommandBufferLevel::ePrimary)
    .setCommandPool(_commandPool)
    .setCommandBufferCount(commandBuffers.size());

  commandBuffers = _device.allocateCommandBuffers(allocInfo);
}

void Renderer::freeCommandBuffers() {
  _device.freeCommandBuffers(_commandPool, commandBuffers);
  commandBuffers.clear();
}

vk::CommandBuffer Renderer::beginFrame() {
  assert(!isFrameStarted && "Can't call beginFrame while already in progress");

  auto result = _swapChain->acquireNextImage(&currentImageIndex);
  if (result == vk::Result::eErrorOutOfDateKHR) {
    recreateSwapChain();
    return nullptr;
  }

  if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
    throw std::runtime_error("failed to acquire swap chain image!");
  }

  isFrameStarted = true;

  auto commandBuffer = getCurrentCommandBuffer();
  vk::CommandBufferBeginInfo beginInfo = vk::CommandBufferBeginInfo();
  commandBuffer.begin(beginInfo);
  return commandBuffer;
}

void Renderer::endFrame() {
  assert(isFrameStarted && "Can't call endFrame while frame is not in progress");
  auto commandBuffer = getCurrentCommandBuffer();
  commandBuffer.end();

  auto result = _swapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
  if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR ||
    _window.wasWindowResized() ) {
      _window.resetWindowResizedFlag();
    recreateSwapChain();
  } else if (result != vk::Result::eSuccess) {
    throw std::runtime_error("failed to present swap chain image!");
  }

  isFrameStarted = false;
  currentFrameIndex = (currentFrameIndex + 1) % _maxFramesInFlight;
}

void Renderer::beginSwapChainRenderPass(vk::CommandBuffer commandBuffer) {
  assert(isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
  assert(
      commandBuffer == getCurrentCommandBuffer() &&
      "Can't begin render pass on command buffer from a different frame");

  vk::Rect2D renderArea = vk::Rect2D({0, 0}, _swapChain->getSwapChainExtent());

  std::array<vk::ClearValue, 2> clearValues{};
  clearValues[0].color = vk::ClearColorValue(0.01f, 0.01f, 1.0f, 1.0f);
  clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);

  vk::RenderPassBeginInfo renderPassInfo = vk::RenderPassBeginInfo()
    .setRenderPass(_swapChain->getRenderPass())
    .setFramebuffer(_swapChain->getFrameBuffer(currentImageIndex))
    .setRenderArea(renderArea)
    .setClearValues(clearValues);

  commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

  vk::Viewport viewport = vk::Viewport()
    .setX(0.0f)
    .setY(0.0f)
    .setWidth(static_cast<float>(_swapChain->getSwapChainExtent().width))
    .setHeight(static_cast<float>(_swapChain->getSwapChainExtent().height))
    .setMinDepth(0.0f)
    .setMaxDepth(1.0f);
  vk::Rect2D scissor{{0, 0}, _swapChain->getSwapChainExtent()};
  commandBuffer.setViewport(0, viewport);
  commandBuffer.setScissor(0, scissor);
}

void Renderer::endSwapChainRenderPass(vk::CommandBuffer commandBuffer) {
  assert(isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress");
  assert(
      commandBuffer == getCurrentCommandBuffer() &&
      "Can't end render pass on command buffer from a different frame");
  commandBuffer.endRenderPass();
}

}  // namespace engine

