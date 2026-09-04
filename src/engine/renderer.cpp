#include "renderer.hpp"
#include <print>

namespace engine {

Renderer::Renderer(
    engine::Window& window, 
    vk::Device const& device,
    vk::PhysicalDevice const & physicalDevice,
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
    _maxFramesInFlight{maxFramesInFlight} {

    std::println("Creating command pool");
    _commandPool = device.createCommandPool({{}, graphicsFamilyIndex});
    recreateSwapChain();
    createCommandBuffers();
}

Renderer::~Renderer() {
  freeCommandBuffers();
  _device.destroyCommandPool(_commandPool);
}

vk::CommandBuffer Renderer::beginFrame(/*bool &hasFrame*/) {
  assert(!_isFrameStarted && "Can't call beginFrame while already in progress");


  vk::ResultValue<uint32_t> nextImageResult = _swapChain->acquireNextImage();
  const vk::Result& vkResult = nextImageResult.result;
  if (vkResult == vk::Result::eErrorOutOfDateKHR) {
    recreateSwapChain();
    //hasFrame = false;
    return VK_NULL_HANDLE;
  }


  if (vkResult != vk::Result::eSuccess && vkResult != vk::Result::eSuboptimalKHR) {
    throw std::runtime_error("failed to acquire swap chain image!");
  }
  _currentImageIndex = nextImageResult.value;
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
  
  auto result = _swapChain->submitCommandBuffer(commandBuffer, _currentImageIndex);
  // TODO: wasWindowResized should be an event rather than a flag that the caller has to reset - this is sloppy
  if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || _window.wasWindowResized()) {
    _window.resetWindowResizedFlag();
    recreateSwapChain();
  } else if (result != vk::Result::eSuccess) {
    throw std::runtime_error("failed to present swap chain image!");
  }

  _isFrameStarted = false;
  _currentFrameIndex = (_currentFrameIndex + 1) % _swapChain->getMaxFramesInFlight();
}

// TODO: Command buffer set up
void Renderer::createCommandBuffers() {
  _commandBuffers.resize(_maxFramesInFlight);

  _commandBuffers = _device.allocateCommandBuffers(
      vk::CommandBufferAllocateInfo(
          _commandPool,
          vk::CommandBufferLevel::ePrimary,
          _commandBuffers.size()
      )
  );

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
      _physicalDevice, 
      _device, 
      _window.getSurface(),
      _window.getExtent(),
      _imageUsageFlags,
      _graphicsFamilyIndex,
      _presentFamilyIndex,
      _maxFramesInFlight,
      nullptr );
    return;
  }
  std::shared_ptr<engine::SwapChain> oldSwapChain = std::move(_swapChain);
  _swapChain = std::make_unique<engine::SwapChain>(
    _physicalDevice, 
    _device, 
    _window.getSurface(),
    _window.getExtent(),
    _imageUsageFlags,
    _graphicsFamilyIndex,
    _presentFamilyIndex,
    _maxFramesInFlight,
    oldSwapChain
  );

  if (!oldSwapChain->compareSwapFormats(*_swapChain.get())) {
    throw std::runtime_error("Swap chain image(or depth) format has changed!");
  }
  
}

}  // namespace engine

