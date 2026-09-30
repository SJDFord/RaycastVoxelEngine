#pragma once

#include "./engine/window.hpp"
#include "graphics/vulkan/physical_device.hpp"

// std lib headers
#include <string>
#include <vector>
#include <memory>

namespace lve {
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

  VkCommandPool getCommandPool() { return commandPool; }

  // TODO: Wrap device functions so that we can delete this getter - no calling code should have access to the underlying Vulkan device
  VkInstance getInstance() { return _instance; }
  VkDevice device() { return _device; }
  VkSurfaceKHR surface() { return surface_; }
  VkQueue graphicsQueue() { return graphicsQueue_; }
  VkQueue presentQueue() { return presentQueue_; }

  SwapChainSupportDetails getSwapChainSupport() { 
      SwapChainSupportDetails details;
      vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physicalDevice, surface_, &details.capabilities);

      uint32_t formatCount;
      vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, surface_, &formatCount, nullptr);

      if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, surface_, &formatCount, details.formats.data());
      }

      uint32_t presentModeCount;
      vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, surface_, &presentModeCount, nullptr);

      if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            _physicalDevice,
            surface_,
            &presentModeCount,
            details.presentModes.data());
      }
      return details;

  };
  uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
  QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(surface_); }
  VkFormat findSupportedFormat(
      const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
for (VkFormat format : candidates) {
  VkFormatProperties props; 
  vkGetPhysicalDeviceFormatProperties(_physicalDevice, format, &props);

  if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
    return format;
  } else if (
      tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
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
  VkCommandBuffer beginSingleTimeCommands();
  void endSingleTimeCommands(VkCommandBuffer commandBuffer);
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
  VkImageView createImageView(VkImage image, VkFormat format);
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
  SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
  QueueFamilyIndices findQueueFamilies(VkSurfaceKHR surface);

  vk::Instance _instance;
  VkDebugUtilsMessengerEXT debugMessenger;
  engine::Window &_window;
  VkCommandPool commandPool;

  VkDevice _device;
  VkSurfaceKHR surface_;
  VkQueue graphicsQueue_;
  VkQueue presentQueue_;

  vk::PhysicalDevice _physicalDevice;

  const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
  const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
};

}  // namespace lve
