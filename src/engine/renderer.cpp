#include "renderer.hpp"
#include <print>

namespace engine {

Renderer::Renderer(
    engine::Window& window, 
    vk::Device device,
    vk::PhysicalDevice physicalDevice,
    vk::ImageUsageFlags        usage,
    uint32_t                   graphicsFamilyIndex,
    uint32_t                   presentFamilyIndex,
    uint32_t                   maxFramesInFlight)
    : _window(window), 
    _device(device), 
    _physicalDevice{physicalDevice}, 
    _imageUsageFlags{usage}, 
    _graphicsFamilyIndex{graphicsFamilyIndex},
    _presentFamilyIndex{presentFamilyIndex},
    _maxFramesInFlight{maxFramesInFlight},
    _isFrameStarted{false} {

    std::println("Creating command pool");
    _commandPool = device.createCommandPool({vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer, graphicsFamilyIndex});
    std::println("Command pool created");
    recreateSwapChain();
    std::println("Swap chain recreated");
    createCommandBuffers();
}

Renderer::~Renderer() {
  freeCommandBuffers();
  _device.destroyCommandPool(_commandPool);
}

vk::CommandBuffer Renderer::beginFrame(/*bool &hasFrame*/) {
  assert(!_isFrameStarted);// && "Can't call beginFrame while already in progress");  
  
  _swapChain->acquireNextImage(&_currentImageIndex);
  /*
  const vk::Result& vkResult = nextImageResult.result;
  if (vkResult == vk::Result::eErrorOutOfDateKHR) {
    recreateSwapChain();
    //hasFrame = false;
    return VK_NULL_HANDLE;
  }
  

  if (vkResult != vk::Result::eSuccess && vkResult != vk::Result::eSuboptimalKHR) {
    throw std::runtime_error("failed to acquire swap chain image!");
  }
    */

  //_currentImageIndex = nextImageResult.value;
  _isFrameStarted = true;

  vk::CommandBuffer commandBuffer = getCurrentCommandBuffer();

  vk::CommandBufferBeginInfo beginInfo = vk::CommandBufferBeginInfo();
  commandBuffer.begin(beginInfo);
  //hasFrame = true;
  return commandBuffer;
}
void Renderer::endFrame() {
  assert(_isFrameStarted && "Can't call endFrame while frame is not in progress");
  auto commandBuffer = getCurrentCommandBuffer();
  commandBuffer.end();
  
  auto result = _swapChain->submitCommandBuffers(&commandBuffer, &_currentImageIndex);
  // TODO: wasWindowResized should be an event rather than a flag that the caller has to reset - this is sloppy
  if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || _window.wasWindowResized()) {
    _window.resetWindowResizedFlag();
    recreateSwapChain();
  } else if (result != vk::Result::eSuccess) {
    throw std::runtime_error("failed to present swap chain image!");
  }

  _isFrameStarted = false;
  _currentFrameIndex = (_currentFrameIndex + 1) % _maxFramesInFlight;
}


vk::RenderPass Renderer::getSwapChainRenderPass() {
  return _swapChain->getRenderPass();
}

void Renderer::beginSwapChainRenderPass(vk::CommandBuffer commandBuffer) {
  assert(_isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
  assert(
      commandBuffer == getCurrentCommandBuffer() &&
      "Can't begin render pass on command buffer from a different frame");

  vk::Extent2D swapChainExtent = _swapChain->getSwapChainExtent();
  vk::Rect2D renderArea = vk::Rect2D({0, 0}, swapChainExtent);
  std::array<vk::ClearValue, 2> clearValues{};
  clearValues[0].color = vk::ClearColorValue(0.01f, 1.0f, 0.01f, 1.0f);
  clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);
  

  vk::RenderPassBeginInfo renderPassInfo = vk::RenderPassBeginInfo()
    .setRenderPass(_swapChain->getRenderPass())
    .setFramebuffer(_swapChain->getFrameBuffer(_currentImageIndex))
    .setRenderArea(renderArea)
    .setClearValues(clearValues);

  commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);


  vk::Viewport viewport = vk::Viewport()
    .setX(0.0f)
    .setY(0.0f)
    .setWidth(swapChainExtent.width)
    .setHeight(swapChainExtent.height)
    .setMinDepth(0.0f)
    .setMaxDepth(1.0f);
  vk::Rect2D scissor{{0, 0}, swapChainExtent};
  commandBuffer.setViewport(0, viewport);
  commandBuffer.setScissor(0, scissor);
}

void Renderer::endSwapChainRenderPass(vk::CommandBuffer commandBuffer) {
  assert(_isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress");
  assert(
      commandBuffer == getCurrentCommandBuffer() &&
      "Can't end render pass on command buffer from a different frame");
  commandBuffer.endRenderPass();
}

// TODO: Command buffer set up
void Renderer::createCommandBuffers() {
  _commandBuffers.resize(_maxFramesInFlight);

  std::println("Command buffers resized");
  _commandBuffers = _device.allocateCommandBuffers(
      vk::CommandBufferAllocateInfo(
          _commandPool,
          vk::CommandBufferLevel::ePrimary,
          _commandBuffers.size()
      )
  );
  std::println("Command buffers created");

  /*
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = lveDevice.getCommandPool();
  allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

  if (vkAllocateCommandBuffers(lveDevice.device(), &allocInfo, commandBuffers.data()) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to allocate command buffers!");
  }
  */
}
void Renderer::freeCommandBuffers() {
  _device.freeCommandBuffers(_commandPool, _commandBuffers);
  _commandBuffers.clear();
}

void Renderer::recreateSwapChain() {
  vk::Extent2D extent = _window.getExtent();
  while (extent.width == 0 || extent.height == 0) {
    extent = _window.getExtent();
    _window.waitEvents();
  }

  _device.waitIdle();

  if (_swapChain == nullptr) {
    _swapChain = std::make_unique<engine::SwapChain>(
      _device,
      _physicalDevice,  
      _window.getSurface(),
      _window.getExtent(),
      /*_imageUsageFlags */
      _graphicsFamilyIndex,
      _presentFamilyIndex,
      _maxFramesInFlight );
    std::printf("Swap Chain PTR %p\n", _swapChain.get());
    std::println("Made swap chain");
    return;
  }
  std::shared_ptr<engine::SwapChain> oldSwapChain = std::move(_swapChain);
  _swapChain = std::make_unique<engine::SwapChain>(
    _device,
    _physicalDevice,  
    _window.getSurface(),
    _window.getExtent(),
    /* _imageUsageFlags, */
    _graphicsFamilyIndex,
    _presentFamilyIndex,
    _maxFramesInFlight,
    oldSwapChain
  );

  std::println("Remade swap chain");
  if (!oldSwapChain->compareSwapFormats(*_swapChain.get())) {
    throw std::runtime_error("Swap chain image(or depth) format has changed!");
  }
  
}

}  // namespace engine

