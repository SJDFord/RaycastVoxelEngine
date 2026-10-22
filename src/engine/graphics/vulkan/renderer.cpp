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

  auto extent = _window.getExtent();
  recreateSwapChain(extent);
  createCommandBuffers();

  _window.addFramebufferResizeListener([&] (vk::Extent2D extent) { 
    std::println("Window resized");
    recreateSwapChain(extent);
  });
}

Renderer::~Renderer() { freeCommandBuffers(); }

void Renderer::recreateSwapChain(vk::Extent2D extent) {
  _device.waitIdle();

  std::println("Recreating swap chain {} x {}", extent.width, extent.height);
  
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
    std::initializer_list<std::function<void(vk::CommandBuffer const &, int)>> renderPassOperations,
    std::function<void(vk::CommandBuffer const &, int)> postRender
  ) {
  auto acquireNextImageResult = _swapChain->acquireNextImage(&_currentImageIndex);
  if (acquireNextImageResult == vk::Result::eErrorOutOfDateKHR) {
    std::println("Out of date KHR after acquiring next image");
    recreateSwapChain(_window.getExtent());
    return false;
  }

  if (acquireNextImageResult != vk::Result::eSuccess && acquireNextImageResult != vk::Result::eSuboptimalKHR) {
    throw std::runtime_error("failed to acquire swap chain image!");
  }

  auto commandBuffer = _commandBuffers[_currentFrameIndex];
  vk::CommandBufferBeginInfo beginInfo = vk::CommandBufferBeginInfo();
  commandBuffer.begin(beginInfo);

  preRender(commandBuffer, _currentFrameIndex);

  for( auto renderPassOperation : renderPassOperations )
  {
    _swapChain->runRenderPass(commandBuffer, _currentFrameIndex, _currentImageIndex, renderPassOperation);
  }

  postRender(commandBuffer, _currentFrameIndex);


  commandBuffer.end();

  auto submitCommandBuffersResult = _swapChain->submitCommandBuffers(&commandBuffer, &_currentImageIndex);
  if (submitCommandBuffersResult == vk::Result::eErrorOutOfDateKHR || submitCommandBuffersResult == vk::Result::eSuboptimalKHR) {
    std::println("Error after submitting command buffers");
    recreateSwapChain(_window.getExtent());
  } else if (submitCommandBuffersResult != vk::Result::eSuccess) {
    throw std::runtime_error("failed to present swap chain image!");
  }

  _currentFrameIndex = (_currentFrameIndex + 1) % _maxFramesInFlight;

  return true;
}

}  // namespace engine

