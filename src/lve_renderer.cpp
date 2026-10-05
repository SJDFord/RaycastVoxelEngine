#include "lve_renderer.hpp"

// std
#include <array>
#include <cassert>
#include <stdexcept>

namespace lve {

LveRenderer::LveRenderer(engine::Window& window, LveDevice& device, uint32_t maxFramesInFlight)
    : _window{window}, lveDevice{device}, _maxFramesInFlight{maxFramesInFlight} {
  recreateSwapChain();
  createCommandBuffers();
}

LveRenderer::~LveRenderer() { freeCommandBuffers(); }

void LveRenderer::recreateSwapChain() {
  auto extent = _window.getExtent();
  while (extent.width == 0 || extent.height == 0) {
    extent = _window.getExtent();
    _window.waitEvents();
  }

  lveDevice.waitIdle();
  
  auto indicies = lveDevice.findPhysicalQueueFamilies();
  if (lveSwapChain == nullptr) {
    lveSwapChain = std::make_unique<LveSwapChain>(
      lveDevice.device(), 
      lveDevice.getPhysicalDevice(), 
      lveDevice.surface(), 
      _window.getExtent(),
      indicies.graphicsFamily, 
      indicies.presentFamily, 
      _maxFramesInFlight 
    );
  } else {
    std::shared_ptr<LveSwapChain> oldSwapChain = std::move(lveSwapChain);
    lveSwapChain = std::make_unique<LveSwapChain>(
      lveDevice.device(), 
      lveDevice.getPhysicalDevice(), 
      lveDevice.surface(), 
      _window.getExtent(),
      indicies.graphicsFamily, 
      indicies.presentFamily, 
      _maxFramesInFlight,
      oldSwapChain
    );

    if (!oldSwapChain->compareSwapFormats(*lveSwapChain.get())) {
      throw std::runtime_error("Swap chain image(or depth) format has changed!");
    }
  }
}

void LveRenderer::createCommandBuffers() {
  commandBuffers.resize(_maxFramesInFlight);

  vk::CommandBufferAllocateInfo allocInfo = vk::CommandBufferAllocateInfo()
    .setLevel(vk::CommandBufferLevel::ePrimary)
    .setCommandPool(lveDevice.getCommandPool())
    .setCommandBufferCount(commandBuffers.size());

  commandBuffers = lveDevice.device().allocateCommandBuffers(allocInfo);
}

void LveRenderer::freeCommandBuffers() {
  lveDevice.device().freeCommandBuffers(lveDevice.getCommandPool(), commandBuffers);
  commandBuffers.clear();
}

vk::CommandBuffer LveRenderer::beginFrame() {
  assert(!isFrameStarted && "Can't call beginFrame while already in progress");

  auto result = lveSwapChain->acquireNextImage(&currentImageIndex);
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

void LveRenderer::endFrame() {
  assert(isFrameStarted && "Can't call endFrame while frame is not in progress");
  auto commandBuffer = getCurrentCommandBuffer();
  commandBuffer.end();

  auto result = lveSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
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

void LveRenderer::beginSwapChainRenderPass(vk::CommandBuffer commandBuffer) {
  assert(isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
  assert(
      commandBuffer == getCurrentCommandBuffer() &&
      "Can't begin render pass on command buffer from a different frame");

  vk::Rect2D renderArea = vk::Rect2D({0, 0}, lveSwapChain->getSwapChainExtent());

  std::array<vk::ClearValue, 2> clearValues{};
  clearValues[0].color = vk::ClearColorValue(0.01f, 0.01f, 1.0f, 1.0f);
  clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);

  vk::RenderPassBeginInfo renderPassInfo = vk::RenderPassBeginInfo()
    .setRenderPass(lveSwapChain->getRenderPass())
    .setFramebuffer(lveSwapChain->getFrameBuffer(currentImageIndex))
    .setRenderArea(renderArea)
    .setClearValues(clearValues);

  commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

  vk::Viewport viewport = vk::Viewport()
    .setX(0.0f)
    .setY(0.0f)
    .setWidth(static_cast<float>(lveSwapChain->getSwapChainExtent().width))
    .setHeight(static_cast<float>(lveSwapChain->getSwapChainExtent().height))
    .setMinDepth(0.0f)
    .setMaxDepth(1.0f);
  vk::Rect2D scissor{{0, 0}, lveSwapChain->getSwapChainExtent()};
  commandBuffer.setViewport(0, viewport);
  commandBuffer.setScissor(0, scissor);
}

void LveRenderer::endSwapChainRenderPass(vk::CommandBuffer commandBuffer) {
  assert(isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress");
  assert(
      commandBuffer == getCurrentCommandBuffer() &&
      "Can't end render pass on command buffer from a different frame");
  commandBuffer.endRenderPass();
}

}  // namespace lve
