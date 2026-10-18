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
  _commandBuffers.resize(_maxFramesInFlight);

  vk::CommandBufferAllocateInfo allocInfo = vk::CommandBufferAllocateInfo()
    .setLevel(vk::CommandBufferLevel::ePrimary)
    .setCommandPool(_commandPool)
    .setCommandBufferCount(_commandBuffers.size());

  _commandBuffers = _device.allocateCommandBuffers(allocInfo);
}

void Renderer::freeCommandBuffers() {
  _device.freeCommandBuffers(_commandPool, _commandBuffers);
  _commandBuffers.clear();
}

bool Renderer::renderFrame(
    std::function<void(vk::CommandBuffer const &, int)> preRender, 
    std::initializer_list<std::function<void(vk::CommandBuffer const &, int)>> renderPasses,
    std::function<void(vk::CommandBuffer const &, int)> postRender
  ) {
  auto acquireNextImageResult = _swapChain->acquireNextImage(&_currentImageIndex);
  if (acquireNextImageResult == vk::Result::eErrorOutOfDateKHR) {
    recreateSwapChain();
    return false;
  }

  if (acquireNextImageResult != vk::Result::eSuccess && acquireNextImageResult != vk::Result::eSuboptimalKHR) {
    throw std::runtime_error("failed to acquire swap chain image!");
  }

  auto commandBuffer = _commandBuffers[_currentFrameIndex];
  vk::CommandBufferBeginInfo beginInfo = vk::CommandBufferBeginInfo();
  commandBuffer.begin(beginInfo);

  preRender(commandBuffer, _currentFrameIndex);

  for( auto renderPass : renderPasses )
  {
    runRenderPass(renderPass);
  }

  postRender(commandBuffer, _currentFrameIndex);


  commandBuffer.end();

  auto submitCommandBuffersResult = _swapChain->submitCommandBuffers(&commandBuffer, &_currentImageIndex);
  if (submitCommandBuffersResult == vk::Result::eErrorOutOfDateKHR || submitCommandBuffersResult == vk::Result::eSuboptimalKHR ||
    _window.wasWindowResized() ) {
    // TODO: resized should be a registered callback rather than polling for it
    _window.resetWindowResizedFlag();
    recreateSwapChain();
  } else if (submitCommandBuffersResult != vk::Result::eSuccess) {
    throw std::runtime_error("failed to present swap chain image!");
  }

  _currentFrameIndex = (_currentFrameIndex + 1) % _maxFramesInFlight;

  return true;
}


void Renderer::runRenderPass(std::function<void(vk::CommandBuffer const &, int)> renderPassOperation) {
  auto commandBuffer = _commandBuffers[_currentFrameIndex];
  vk::Rect2D renderArea = vk::Rect2D({0, 0}, _swapChain->getSwapChainExtent());

  std::array<vk::ClearValue, 2> clearValues{};
  clearValues[0].color = vk::ClearColorValue(0.01f, 0.01f, 1.0f, 1.0f);
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
    .setWidth(static_cast<float>(_swapChain->getSwapChainExtent().width))
    .setHeight(static_cast<float>(_swapChain->getSwapChainExtent().height))
    .setMinDepth(0.0f)
    .setMaxDepth(1.0f);
  vk::Rect2D scissor{{0, 0}, _swapChain->getSwapChainExtent()};
  commandBuffer.setViewport(0, viewport);
  commandBuffer.setScissor(0, scissor);

  renderPassOperation(commandBuffer, _currentFrameIndex);

  commandBuffer.endRenderPass();
}

}  // namespace engine

