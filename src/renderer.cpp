#include "renderer.hpp"

// std
#include <array>
#include <cassert>
#include <stdexcept>

Renderer::Renderer(Window& window, std::shared_ptr<Device> device)
    : window{window}, device{device} {
  recreateSwapChain();
  createCommandBuffers();
}

Renderer::~Renderer() { freeCommandBuffers(); }

void Renderer::recreateSwapChain() {
  auto extent = window.getExtent();
  while (extent.width == 0 || extent.height == 0) {
    extent = window.getExtent();
    glfwWaitEvents();
  }
  device->waitIdle();
  
  if (swapChain == nullptr) {
    swapChain = std::make_unique<SwapChain>(device, extent);
  } else {
    std::shared_ptr<SwapChain> oldSwapChain = std::move(swapChain);
    swapChain = std::make_unique<SwapChain>(device, extent, oldSwapChain);

    if (!oldSwapChain->compareSwapFormats(*swapChain.get())) {
      throw std::runtime_error("Swap chain image(or depth) format has changed!");
    }
  }
}

void Renderer::createCommandBuffers() {
  //commandBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT, vk::raii::CommandBuffer(nullptr));

  vk::CommandBufferAllocateInfo allocInfo = vk::CommandBufferAllocateInfo()
      .setLevel(vk::CommandBufferLevel::ePrimary)
      .setCommandPool(device->getCommandPool())
          .setCommandBufferCount(static_cast<uint32_t>(SwapChain::MAX_FRAMES_IN_FLIGHT));

  // TODO: Encapsulate
  
  commandBuffers = vk::raii::CommandBuffers(device->device(), allocInfo);

  // TODO: Encapsulate
  //commandBuffers = device->device().allocateCommandBuffers(allocInfo);
}

void Renderer::freeCommandBuffers() {
  //TODO
}

const vk::raii::CommandBuffer& Renderer::beginFrame() {
  assert(!isFrameStarted && "Can't call beginFrame while already in progress");

  std::pair<vk::Result, uint32_t> resultPair = swapChain->acquireNextImage();
  const vk::Result& result = resultPair.first;
  if (result == vk::Result::eErrorOutOfDateKHR) {
    recreateSwapChain();
    return VK_NULL_HANDLE;
  }

  if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
    throw std::runtime_error("failed to acquire swap chain image!");
  }
  currentImageIndex = resultPair.second;
  isFrameStarted = true;

  const vk::raii::CommandBuffer& commandBuffer = getCurrentCommandBuffer();
  vk::CommandBufferBeginInfo beginInfo = vk::CommandBufferBeginInfo();
  commandBuffer.begin(beginInfo);
  return commandBuffer;
}

void Renderer::endFrame() {
  assert(isFrameStarted && "Can't call endFrame while frame is not in progress");
  const vk::raii::CommandBuffer& commandBuffer = getCurrentCommandBuffer();
  commandBuffer.end();

  auto result = swapChain->submitCommandBuffers(commandBuffer, currentImageIndex);
  if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR ||
      window.wasWindowResized()) {
    window.resetWindowResizedFlag();
    recreateSwapChain();
  } else if (result != vk::Result::eSuccess) {
    throw std::runtime_error("failed to present swap chain image!");
  }

  isFrameStarted = false;
  currentFrameIndex = (currentFrameIndex + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT;
}

void Renderer::beginSwapChainRenderPass(const vk::raii::CommandBuffer& commandBuffer) {
  assert(isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");

  vk::Rect2D renderArea =
      vk::Rect2D().setOffset(vk::Offset2D(0, 0)).setExtent(swapChain->getSwapChainExtent());
  vk::RenderPassBeginInfo renderPassInfo = vk::RenderPassBeginInfo()
      .setRenderPass(swapChain->getRenderPass())
      .setFramebuffer(swapChain->getFrameBuffer(currentImageIndex))
      .setRenderArea(renderArea);

  std::array<vk::ClearValue, 2> clearValues{
      vk::ClearValue().setColor({0.01f, 0.01f, 0.01f, 1.0f}),
      vk::ClearValue().setDepthStencil(vk::ClearDepthStencilValue(1.0f, 0))
  };
  renderPassInfo.setClearValueCount(static_cast<uint32_t>(clearValues.size()))
      .setClearValues(clearValues);

  commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

  vk::Viewport viewport = vk::Viewport()
    .setX(0.0f)
    .setY(0.0f)
    .setWidth(static_cast<float>(swapChain->getSwapChainExtent().width))
    .setHeight(static_cast<float>(swapChain->getSwapChainExtent().height))
    .setMinDepth(0.0f)
    .setMaxDepth(1.0f);

  vk::Rect2D scissor = vk::Rect2D(vk::Offset2D(0, 0), swapChain->getSwapChainExtent());
  commandBuffer.setViewport(0, viewport);
  commandBuffer.setScissor(0, scissor);
}

void Renderer::endSwapChainRenderPass(const vk::raii::CommandBuffer& commandBuffer) {
  assert(isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress");

  commandBuffer.endRenderPass();
}
