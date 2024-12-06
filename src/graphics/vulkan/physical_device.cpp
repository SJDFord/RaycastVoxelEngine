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

  /*
  vkGetPhysicalDeviceProperties(physicalDevice, &properties);
  std::cout << "physical device: " << properties.deviceName << std::endl;
    */
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

PhysicalDevice::~PhysicalDevice() {
    // TODO: Put device back in devices list
}
