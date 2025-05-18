#pragma once

#include "window.hpp"

// std lib headers
#include <string>
#include <vector>
#include <memory>

struct SwapChainSupportInfo {
  vk::SurfaceCapabilitiesKHR capabilities;
  std::vector<vk::SurfaceFormatKHR> formats;
  std::vector<vk::PresentModeKHR> presentModes;
};

class Device {
 public:
  Device(Window& window);
  ~Device();

  // Not copyable or movable
  Device(const Device&) = delete;
  Device& operator=(const Device&) = delete;
  Device(Device&&) = delete;
  Device& operator=(Device&&) = delete;

  vk::raii::CommandPool& getCommandPool() { return _commandPool; }
  vk::raii::Device& device() { return _device; }
  vk::raii::SurfaceKHR& surface() { return _surface; }
  vk::raii::Queue graphicsQueue() { return _graphicsQueue; }
  vk::raii::Queue presentQueue() { return _presentQueue; }

  SwapChainSupportInfo getSwapChainSupport();
  uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
  int getGraphicsQueueFamily() { return _graphicsQueueIndex; };
  int getPresentQueueFamily() { return _presentQueueIndex; };
  vk::PhysicalDeviceProperties getPhysicalDeviceProperties() { return _properties; };
  vk::PhysicalDeviceFeatures getPhysicalDeviceFeatures() { return _features; };
  vk::PhysicalDeviceMemoryProperties getPhysicalDeviceMemoryProperties() { return _memoryProperties; };


  vk::Format findSupportedFormat(
      const std::vector<vk::Format>& candidates,
      vk::ImageTiling tiling,
      vk::FormatFeatureFlags features);
  // Buffer Helper Functions
  /*
  void createBuffer(
      VkDeviceSize size,
      VkBufferUsageFlags usage,
      VkMemoryPropertyFlags properties,
      VkBuffer &buffer,
      VkDeviceMemory &bufferMemory);
  */ 
  vk::raii::CommandBuffer beginSingleTimeCommands();
  void endSingleTimeCommands(vk::raii::CommandBuffer& commandBuffer);
  void copyBuffer(
      const vk::raii::Buffer& srcBuffer, const vk::raii::Buffer& dstBuffer, vk::DeviceSize size);
  /*
  void copyBufferToImage(
      VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);

  void createImageWithInfo(
      const VkImageCreateInfo &imageInfo,
      VkMemoryPropertyFlags properties,
      VkImage &image,
      VkDeviceMemory &imageMemory);
  */

 private:
  void createInstance();
  void setupDebugMessenger();
  void createSurface();
  void createPhysicalDevice(); 
  void createLogicalDevice();
  void createCommandPool();

  // helper functions
  bool isDeviceSuitable(vk::raii::PhysicalDevice device);
  std::vector<const char *> getRequiredExtensions();
  bool checkValidationLayerSupport();
  void populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT &createInfo);
  void hasGflwRequiredInstanceExtensions();
  bool checkDeviceExtensionSupport(vk::raii::PhysicalDevice device);

  vk::DebugUtilsMessengerEXT _debugMessenger;
  Window& _window;
  vk::raii::Context _context;
  vk::raii::Instance _instance = VK_NULL_HANDLE;
  vk::raii::SurfaceKHR _surface = VK_NULL_HANDLE;
  vk::raii::PhysicalDevice _physicalDevice = VK_NULL_HANDLE;
  vk::PhysicalDeviceProperties _properties;
  vk::PhysicalDeviceFeatures _features;
  vk::PhysicalDeviceMemoryProperties _memoryProperties;
  vk::raii::Device _device = VK_NULL_HANDLE;  
  vk::raii::Queue _graphicsQueue = VK_NULL_HANDLE;
  vk::raii::Queue _presentQueue = VK_NULL_HANDLE;
  vk::raii::CommandPool _commandPool = VK_NULL_HANDLE;
  int _graphicsQueueIndex = -1;
  int _presentQueueIndex = -1;

  const std::vector<const char *> _validationLayers = {"VK_LAYER_KHRONOS_validation"};
  const std::vector<const char *> _deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
};