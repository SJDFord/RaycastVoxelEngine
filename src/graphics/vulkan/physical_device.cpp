#include "physical_device.hpp"

#include <iostream>
#include <set>
#include <map>
#include <stdexcept>

// See https://github.com/fuchstraumer/VulpesRender/blob/master/core/src/LogicalDevice.cpp
static std::map<int32_t, VkPhysicalDevice> physicalDevices;

PhysicalDevice::PhysicalDevice(const VkInstance& instance) {
  if (physicalDevices.empty()) {
    ReadPhysicalDevices(instance);
  }

  physicalDevice = GetBestAvailPhysicalDevice();

  if (physicalDevice == VK_NULL_HANDLE) {
    throw std::runtime_error("failed to find a suitable GPU!");
  }

  getAttributes();
  retrieveQueueFamilyProperties();
  //VULKAN_HPP_NAMESPACE::ApplicationInfo temp;
  //vk::ApplicationInfo temp;
  //vk::raii::PhysicalDevice p;
  vkGetPhysicalDeviceProperties(physicalDevice, &properties);
  std::cout << "physical device: " << properties.deviceName << std::endl;
}
void PhysicalDevice::ReadPhysicalDevices(const VkInstance& instance) {
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
  if (deviceCount == 0) {
    throw std::runtime_error("failed to find GPUs with Vulkan support!");
  }
  std::cout << "Device count: " << deviceCount << std::endl;
  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

  for (const auto& device : devices) {
    physicalDevices.emplace(ScoreDevice(device), device);
  }
}

void PhysicalDevice::createLogicalDevice(const VkDeviceCreateInfo* createInfo, VkDevice* pDevice) {
  if (vkCreateDevice(physicalDevice, createInfo, nullptr, pDevice) != VK_SUCCESS) {
    throw std::runtime_error("failed to create logical device!");
  }
}

int32_t PhysicalDevice::ScoreDevice(const VkPhysicalDevice& device) {
  int32_t score = 0;
  VkPhysicalDeviceFeatures features;
  VkPhysicalDeviceProperties properties;
  vkGetPhysicalDeviceFeatures(device, &features);
  vkGetPhysicalDeviceProperties(device, &properties);

  if (!features.geometryShader) {
    score -= 1000;
  }

  if (!features.tessellationShader) {
    score -= 1000;
  }

  if (!features.samplerAnisotropy) {
    score -= 250;
  }

  if (!features.imageCubeArray) {
    score -= 250;
  }

  if (!features.fullDrawIndexUint32) {
    score -= 500;
  }

  if (features.multiDrawIndirect) {
    score += 250;
  }

  if (features.textureCompressionETC2) {
    score += 250;
  }

  if (features.textureCompressionASTC_LDR) {
    score += 250;
  }

  score += (10 * properties.limits.maxBoundDescriptorSets);
  score +=
      (5 * (properties.limits.maxPerStageDescriptorUniformBuffers +
            properties.limits.maxPerStageDescriptorSampledImages +
            properties.limits.maxPerStageDescriptorSamplers +
            properties.limits.maxPerStageDescriptorStorageImages));

  if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
    score += 3000;
  }

  auto tex_sizes = {
      properties.limits.maxImageDimension1D,
      properties.limits.maxImageDimension2D,
      properties.limits.maxImageDimension3D,
  };

  for (auto& sz : tex_sizes) {
    score += sz / 100;
  }

  return score;
}

VkPhysicalDevice PhysicalDevice::GetBestAvailPhysicalDevice() {
  auto best_device = (physicalDevices.rbegin());
  VkPhysicalDevice result = best_device->second;
  physicalDevices.erase(best_device->first);
  return result;
}

void PhysicalDevice::getAttributes() {
  vkGetPhysicalDeviceProperties(physicalDevice, &properties);
  vkGetPhysicalDeviceFeatures(physicalDevice, &features);
  vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);
}

void PhysicalDevice::retrieveQueueFamilyProperties() {
  uint32_t queue_family_cnt = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queue_family_cnt, nullptr);
  
  queueFamilyProperties.resize(queue_family_cnt);
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queue_family_cnt, queueFamilyProperties.data());
}

QueueFamilyIndices PhysicalDevice::findQueueFamilies(VkSurfaceKHR surface) {
  QueueFamilyIndices indices;

  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

  int i = 0;
  for (const auto &queueFamily : queueFamilies) {
    if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphicsFamily = i;
      indices.graphicsFamilyHasValue = true;
    }
    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
    if (queueFamily.queueCount > 0 && presentSupport) {
      indices.presentFamily = i;
      indices.presentFamilyHasValue = true;
    }
    if (indices.isComplete()) {
      break;
    }

    i++;
  }

  return indices;
}

SwapChainSupportDetails PhysicalDevice::querySwapChainSupport(VkSurfaceKHR surface) {
  SwapChainSupportDetails details;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities);

  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);

  if (formatCount != 0) {
    details.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, details.formats.data());
  }

  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

  if (presentModeCount != 0) {
    details.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        physicalDevice,
        surface,
        &presentModeCount,
        details.presentModes.data());
  }
  return details;
}

VkFormat PhysicalDevice::findSupportedFormat(
  const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
for (VkFormat format : candidates) {
  VkFormatProperties props;
  vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

  if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
    return format;
  } else if (
      tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
    return format;
  }
}
throw std::runtime_error("failed to find supported format!");
}

uint32_t PhysicalDevice::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) &&
        (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }

  throw std::runtime_error("failed to find suitable memory type!");
}

PhysicalDevice::~PhysicalDevice() {
    // TODO: Put device back in devices list
    std::cout << "releasing physical device: " << properties.deviceName << std::endl;
}
