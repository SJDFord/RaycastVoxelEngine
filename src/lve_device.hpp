#pragma once

#include "./engine/window.hpp"

// std lib headers
#include <string>
#include <vector>
#include <memory>

namespace lve {
struct QueueFamilyIndices {
    uint32_t graphicsFamily;
    uint32_t presentFamily;
    bool graphicsFamilyHasValue = false;
    bool presentFamilyHasValue = false;
    bool isComplete() { return graphicsFamilyHasValue && presentFamilyHasValue; }
};
struct SwapChainSupportDetails {
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
};

class LveDevice {
 public:
#ifdef NDEBUG
  const bool enableValidationLayers = false;
#else
  const bool enableValidationLayers = false;
#endif

  LveDevice(engine::Window &window);
  ~LveDevice();

  // Not copyable or movable
  LveDevice(const LveDevice &) = delete;
  LveDevice &operator=(const LveDevice &) = delete;
  LveDevice(LveDevice &&) = delete;
  LveDevice &operator=(LveDevice &&) = delete;

  vk::CommandPool getCommandPool() { return _commandPool; }

  // TODO: Wrap device functions so that we can delete this getter - no calling code should have access to the underlying Vulkan device
  VkInstance getInstance() { return _instance; }
  vk::Device device() { return _device; }
  VkSurfaceKHR surface() { return _surface; }
  VkQueue graphicsQueue() { return _graphicsQueue; }
  VkQueue presentQueue() { return _presentQueue; }

  SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport(); };
  uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
  QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(_surface); }
  vk::Format findSupportedFormat(
      const std::vector<vk::Format> &candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) {
for (vk::Format format : candidates) {
  auto props = _physicalDevice.getFormatProperties(format);

  if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
    return format;
  } else if (
      tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
    return format;
  }
}
throw std::runtime_error("failed to find supported format!");
      };

  // Buffer Helper Functions
  void createBuffer(
      VkDeviceSize size,
      VkBufferUsageFlags usage,
      VkMemoryPropertyFlags properties,
      VkBuffer &buffer,
      VkDeviceMemory &bufferMemory);
  vk::CommandBuffer beginSingleTimeCommands();
  void endSingleTimeCommands(vk::CommandBuffer commandBuffer);
  void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
  void copyBufferToImage(
      VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);
  void transitionImageLayout(
      VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

  void createImageWithInfo(
      const VkImageCreateInfo &imageInfo,
      VkMemoryPropertyFlags properties,
      VkImage &image,
      VkDeviceMemory &imageMemory);
  VkImageView createImageView(vk::Image image, vk::Format format);
  VkSampler createSampler();

  void waitIdle();

  void destroyShaderModule(VkShaderModule shaderModule);
  void destroyPipeline(VkPipeline pipeline);
  void destroyPipelineLayout(VkPipelineLayout pipelineLayout);

  void destroySampler(VkSampler sampler);
  void destroyImageView(VkImageView imageView);
  void destroyImage(VkImage image);
  void freeMemory(VkDeviceMemory deviceMemory);

  VkPhysicalDeviceProperties properties;

 private:
  void createInstance();
  void createSurface();
  //void pickPhysicalDevice();
  void createLogicalDevice();
  void createCommandPool();

  // helper functions
  bool isDeviceSuitable(VkPhysicalDevice device);
  bool checkDeviceExtensionSupport(VkPhysicalDevice device);
  SwapChainSupportDetails querySwapChainSupport();
  QueueFamilyIndices findQueueFamilies(vk::SurfaceKHR surface);

  vk::Instance _instance;
  VkDebugUtilsMessengerEXT debugMessenger;
  engine::Window &_window;
  vk::CommandPool _commandPool;

  vk::Device _device;
  vk::SurfaceKHR _surface;
  vk::Queue _graphicsQueue;
  vk::Queue _presentQueue;

  vk::PhysicalDevice _physicalDevice;

  const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
  const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
};

}  // namespace lve
