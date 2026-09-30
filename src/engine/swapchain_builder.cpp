#include "swapchain_builder.hpp"

namespace engine {

SwapchainBuilder::SwapchainBuilder(vk::Device const& device) {
  _device = device;
  _oldSwapchain = VK_NULL_HANDLE;
}

SwapchainBuilder::~SwapchainBuilder() {}


SwapchainBuilder& SwapchainBuilder::setSurfaceFormat(vk::SurfaceFormatKHR surfaceFormat) {
    _surfaceFormat = surfaceFormat;
    return *this;
}

SwapchainBuilder& SwapchainBuilder::setPresentMode(vk::PresentModeKHR presentMode) {
    _presentMode = presentMode;
    return *this;
}
SwapchainBuilder& SwapchainBuilder::setExtent(vk::Extent2D extent) {
    _extent = extent;
    return *this;
}
SwapchainBuilder& SwapchainBuilder::setGraphicsQueueFamilyIndex(uint32_t queueFamilyIndex) {
    _graphicsQueueFamilyIndex = queueFamilyIndex;
    return *this;
}
SwapchainBuilder& SwapchainBuilder::setPresentQueueFamilyIndex(uint32_t queueFamilyIndex) {
    _presentQueueFamilyIndex = queueFamilyIndex;
    return *this;
}
SwapchainBuilder& SwapchainBuilder::setOldSwapchain(vk::SwapchainKHR oldSwapchain) {
    _oldSwapchain = oldSwapchain;
    return *this;
}

SwapchainBuilder& SwapchainBuilder::setSurfaceCapabilities(vk::SurfaceCapabilitiesKHR capabilities) {
    _capabilities = capabilities;
    return *this;
}
SwapchainBuilder& SwapchainBuilder::setSurface(vk::SurfaceKHR surface) {
    _surface = surface;
    return *this;
}

vk::SwapchainKHR SwapchainBuilder::build() {
  uint32_t imageCount = _capabilities.minImageCount + 1;
  if (_capabilities.maxImageCount > 0 &&
      imageCount > _capabilities.maxImageCount) {
    imageCount = _capabilities.maxImageCount;
  }
  // TODO: This needs a builder - we are doing too much here
  vk::SwapchainCreateInfoKHR createInfo = vk::SwapchainCreateInfoKHR()
    .setSurface(_surface)
    .setMinImageCount(imageCount)
    .setImageFormat(_surfaceFormat.format)
    .setImageColorSpace(_surfaceFormat.colorSpace)
    .setImageExtent(_extent)
    .setImageArrayLayers(1)
    .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);

  uint32_t queueFamilyIndices[] = {_graphicsQueueFamilyIndex, _presentQueueFamilyIndex};

  if (_graphicsQueueFamilyIndex != _presentQueueFamilyIndex) {
    createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    createInfo.imageSharingMode = vk::SharingMode::eExclusive;
    createInfo.queueFamilyIndexCount = 0;      // Optional
    createInfo.pQueueFamilyIndices = nullptr;  // Optional
  }

  createInfo.preTransform = _capabilities.currentTransform;
  createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;

  createInfo.presentMode = _presentMode;
  createInfo.clipped = VK_TRUE;
  createInfo.oldSwapchain = _oldSwapchain;

  auto swapchain = _device.createSwapchainKHR(createInfo);
  return swapchain;
}

}  // namespace engine

