#include "device.hpp"

// std headers
#include <cstring>
#include <iostream>
#include <set>
#include <unordered_set>


// local callback functions
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData) {
  std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
  
  return VK_FALSE;
}
/*
vk::Result CreateDebugUtilsMessengerEXT(
    vk::raii::Instance instance,
    const vk::DebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const vk::AllocationCallbacks *pAllocator,
    vk::DebugUtilsMessengerEXT *pDebugMessenger) {
  
  auto func = instance.getProcAddr("vkCreateDebugUtilsMessengerEXT");

  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance,
      "vkCreateDebugUtilsMessengerEXT");
  
  if (func != nullptr) {
    return func(*(*instance), pCreateInfo, pAllocator, pDebugMessenger);
  } else {
    return vk::Result::eErrorExtensionNotPresent;
  }
}
*/

void DestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks *pAllocator) {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance,
      "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr) {
    func(instance, debugMessenger, pAllocator);
  }
}

// class member functions
Device::Device(Window &window) : _window{window}, _context{} {
  createInstance();
  setupDebugMessenger();
  createSurface();
  createPhysicalDevice();
  createLogicalDevice();
  createCommandPool();
}

Device::~Device() {
}

void Device::createInstance() {
  const auto validationLayerProperties = vk::enumerateInstanceLayerProperties();

  for (const char *layerName : _validationLayers) {
    bool layerFound = false;

    for (const auto &layerProperties : validationLayerProperties) {
      if (strcmp(layerName, layerProperties.layerName) == 0) {
        layerFound = true;
        break;
      }
    }

    if (!layerFound) {
      throw std::string("Failed to find layer: ") + layerName;
      return;
    }
  }

  vk::ApplicationInfo appInfo = vk::ApplicationInfo()
    .setPApplicationName("Raycast Voxel Engine")
    .setApplicationVersion(VK_MAKE_VERSION(1, 0, 0))
    .setPEngineName("No Engine")
    .setApiVersion(VK_API_VERSION_1_0);

  auto glfwExtensionCount = 0u;
  auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
  std::vector<const char *> glfwExtensionsVector(
      glfwExtensions,
      glfwExtensions + glfwExtensionCount);

  std::string ex = "VK_EXT_debug_utils";
  glfwExtensionsVector.push_back(ex.c_str());

  const vk::InstanceCreateInfo instanceCreateInfo =
      vk::InstanceCreateInfo()
          .setFlags(vk::InstanceCreateFlags())
          .setPApplicationInfo(&appInfo)
          .setEnabledLayerCount(static_cast<uint32_t>(_validationLayers.size()))
          .setPpEnabledLayerNames(_validationLayers.data())
          .setEnabledExtensionCount(static_cast<uint32_t>(glfwExtensionsVector.size()))
          .setPpEnabledExtensionNames(glfwExtensionsVector.data());

  _instance = vk::raii::Instance(_context, instanceCreateInfo);
}

void Device::createCommandPool() {
  vk::CommandPoolCreateInfo poolInfo = vk::CommandPoolCreateInfo()
      .setQueueFamilyIndex(_graphicsQueueIndex)
      .setFlags(
          vk::CommandPoolCreateFlagBits::eTransient | 
          vk::CommandPoolCreateFlagBits::eResetCommandBuffer
      );
  
  _commandPool = _device.createCommandPool(poolInfo);
}


void Device::waitIdle() { _device.waitIdle(); }

void Device::createSurface() { 
    _surface = _window.createWindowSurface(_instance);
}

void Device::createPhysicalDevice() {
  const auto physicalDevices = _instance.enumeratePhysicalDevices();

  for (const auto &physDev : physicalDevices) {
    const auto props = physDev.getProperties();
    std::cout << "Found Physical Device: " << props.deviceName << std::endl;
  }

  _physicalDevice = physicalDevices[0];
  _properties = _physicalDevice.getProperties();
  _features = _physicalDevice.getFeatures();
  _memoryProperties = _physicalDevice.getMemoryProperties();
  std::cout << "Picking Physical Device : " << _properties.deviceName << std::endl;
}

void Device::createLogicalDevice() {
  const auto queueFamilyProperties = _physicalDevice.getQueueFamilyProperties();

  for (int i = 0; i < static_cast<int>(queueFamilyProperties.size()); i++) {
    if (queueFamilyProperties[i].queueCount > 0 &&
        queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics) {
      _graphicsQueueIndex = i;
    }
    const auto presentSupport = _physicalDevice.getSurfaceSupportKHR(i, _surface);
    if (queueFamilyProperties[i].queueCount > 0 && presentSupport) {
      _presentQueueIndex = i;
      break;
    }
  }

  std::cout << "Found Graphics Index : " << std::to_string(_graphicsQueueIndex)
            << "and Present Index: " << std::to_string(_presentQueueIndex) << std::endl;

  std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueQueueFamilies = {
      static_cast<uint32_t>(_graphicsQueueIndex),
      static_cast<uint32_t>(_presentQueueIndex)};

  float queuePriority = 1.0f;
  for (uint32_t queueFamily : uniqueQueueFamilies) {
    vk::DeviceQueueCreateInfo queueCreateInfo = vk::DeviceQueueCreateInfo()
                                                    .setQueueFamilyIndex(queueFamily)
                                                    .setQueueCount(1)
                                                    .setPQueuePriorities(&queuePriority);
    queueCreateInfos.push_back(queueCreateInfo);
  }

  vk::PhysicalDeviceFeatures deviceFeatures =
      vk::PhysicalDeviceFeatures().setSamplerAnisotropy(true);

  vk::DeviceCreateInfo createInfo =
      vk::DeviceCreateInfo()
          .setQueueCreateInfoCount(static_cast<uint32_t>(queueCreateInfos.size()))
          .setPQueueCreateInfos(queueCreateInfos.data())
          .setPEnabledFeatures(&deviceFeatures)
          .setEnabledExtensionCount(static_cast<uint32_t>(_deviceExtensions.size()))
          .setPpEnabledExtensionNames(_deviceExtensions.data())
          .setEnabledLayerCount(static_cast<uint32_t>(_validationLayers.size()))
          .setPpEnabledLayerNames(_validationLayers.data());

  _device = _physicalDevice.createDevice(createInfo);
  _graphicsQueue = _device.getQueue(_graphicsQueueIndex, 0);
  _presentQueue = _device.getQueue(_presentQueueIndex, 0);
}

void Device::populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT &createInfo) {
  createInfo = vk::DebugUtilsMessengerCreateInfoEXT()
    .setMessageSeverity(
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
    .setMessageType(
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance)
    .setPfnUserCallback(debugCallback)
    .setPUserData(nullptr);
}

void Device::setupDebugMessenger() {
  vk::DebugUtilsMessengerCreateInfoEXT createInfo;
  populateDebugMessengerCreateInfo(createInfo);
  /*
  if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
    throw std::runtime_error("failed to set up debug messenger!");
  }
  */
}

std::vector<const char *> Device::getRequiredExtensions() {
  uint32_t glfwExtensionCount = 0;
  const char **glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

  //if (enableValidationLayers) {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  //}

  return extensions;
}

void Device::hasGflwRequiredInstanceExtensions() {
  std::vector<vk::ExtensionProperties> extensions = vk::enumerateInstanceExtensionProperties();

  std::cout << "available extensions:" << std::endl;
  std::unordered_set<std::string> available;
  for (const auto &extension : extensions) {
    std::cout << "\t" << extension.extensionName << std::endl;
    available.insert(extension.extensionName);
  }

  std::cout << "required extensions:" << std::endl;
  auto requiredExtensions = getRequiredExtensions();
  for (const auto &required : requiredExtensions) {
    std::cout << "\t" << required << std::endl;
    if (available.find(required) == available.end()) {
      throw std::runtime_error("Missing required glfw extension");
    }
  }
}

/*
bool LveDevice::checkDeviceExtensionSupport(VkPhysicalDevice device) {
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(
      device,
      nullptr,
      &extensionCount,
      availableExtensions.data());

  std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

  for (const auto &extension : availableExtensions) {
    requiredExtensions.erase(extension.extensionName);
  }

  return requiredExtensions.empty();
}
*/

SwapChainSupportInfo Device::getSwapChainSupport() {
  SwapChainSupportInfo info = SwapChainSupportInfo();
  info.capabilities = _physicalDevice.getSurfaceCapabilitiesKHR(_surface);
  info.formats = _physicalDevice.getSurfaceFormatsKHR(_surface);
  info.presentModes = _physicalDevice.getSurfacePresentModesKHR(_surface);
  return info;
}

uint32_t Device::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
  for (uint32_t i = 0; i < _memoryProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) &&
        (_memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }

  throw std::runtime_error("failed to find suitable memory type!");
}

vk::Format Device::findSupportedFormat(
    const std::vector<vk::Format> &candidates,
    vk::ImageTiling tiling,
    vk::FormatFeatureFlags features) {
  for (vk::Format format : candidates) {
    
      
    vk::FormatProperties props = _physicalDevice.getFormatProperties(format);
    if (tiling ==  vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
      return format;
    } else if (
        tiling == vk::ImageTiling::eOptimal &&
        (props.optimalTilingFeatures & features) == features) {
      return format;
    }
  }
  throw std::runtime_error("failed to find supported format!");
}

/*
void LveDevice::createBuffer(
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer &buffer,
    VkDeviceMemory &bufferMemory) {
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(device_, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to create vertex buffer!");
  }

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(device_, buffer, &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

  if (vkAllocateMemory(device_, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate vertex buffer memory!");
  }

  vkBindBufferMemory(device_, buffer, bufferMemory, 0);
}
*/

vk::raii::CommandBuffer Device::beginSingleTimeCommands() {
  vk::CommandBufferAllocateInfo allocInfo = vk::CommandBufferAllocateInfo()
      .setLevel(vk::CommandBufferLevel::ePrimary)
      .setCommandPool(_commandPool)
      .setCommandBufferCount(1);

  std::vector<vk::raii::CommandBuffer> commandBuffers = _device.allocateCommandBuffers(allocInfo);
  if (commandBuffers.size() < 1) {
    throw "Failed to allocate command buffer";
  }
  vk::raii::CommandBuffer commandBuffer(std::move(commandBuffers[0]));

  vk::CommandBufferBeginInfo beginInfo = vk::CommandBufferBeginInfo()
      .setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

  commandBuffer.begin(beginInfo);

  return commandBuffer;
}

void Device::endSingleTimeCommands(vk::raii::CommandBuffer& commandBuffer) {
  commandBuffer.end();
  
  vk::SubmitInfo submitInfo =
      vk::SubmitInfo().setCommandBufferCount(1).setPCommandBuffers(&(*commandBuffer));

  _graphicsQueue.submit(submitInfo);
  _graphicsQueue.waitIdle();
  
  commandBuffer.release();
  //vkFreeCommandBuffers(device_, commandPool, 1, &commandBuffer);
}



void Device::copyBuffer(
    const vk::raii::Buffer &srcBuffer, const vk::raii::Buffer &dstBuffer, vk::DeviceSize size) {
  vk::raii::CommandBuffer commandBuffer = beginSingleTimeCommands();

  vk::BufferCopy copyRegion{};
  copyRegion.srcOffset = 0;  // Optional
  copyRegion.dstOffset = 0;  // Optional
  copyRegion.size = size;
  commandBuffer.copyBuffer(srcBuffer, dstBuffer, copyRegion);

  endSingleTimeCommands(commandBuffer);
}
/*
void LveDevice::copyBufferToImage(
    VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount) {
  VkCommandBuffer commandBuffer = beginSingleTimeCommands();

  VkBufferImageCopy region{};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;

  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = layerCount;

  region.imageOffset = {0, 0, 0};
  region.imageExtent = {width, height, 1};

  vkCmdCopyBufferToImage(
      commandBuffer,
      buffer,
      image,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      1,
      &region);
  endSingleTimeCommands(commandBuffer);
}
*/
void Device::createImageWithInfo(
    const vk::ImageCreateInfo &imageInfo,
    vk::MemoryPropertyFlags properties,
    vk::raii::Image &image,
    vk::raii::DeviceMemory &imageMemory) {

  image = _device.createImage(imageInfo);

 
  vk::MemoryRequirements memRequirements = image.getMemoryRequirements();

  vk::MemoryAllocateInfo allocInfo =
      vk::MemoryAllocateInfo()
          .setAllocationSize(memRequirements.size)
          .setMemoryTypeIndex(findMemoryType(memRequirements.memoryTypeBits, properties));
  
  imageMemory = _device.allocateMemory(allocInfo);
  image.bindMemory(imageMemory, 0);
}



  void Device::destroyPipelineLayout(VkPipelineLayout pipelineLayout) {
  //vkDestroyPipelineLayout(_device.get, pipelineLayout, nullptr);
  }