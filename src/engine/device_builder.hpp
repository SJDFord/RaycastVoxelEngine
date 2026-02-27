#pragma once

#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <iostream>
#include <string>
#include <vector>

namespace engine {

class DeviceBuilder {
 public:
  DeviceBuilder(const vk::PhysicalDevice& physicalDevice, uint32_t queueFamilyIndex);
  ~DeviceBuilder();

  DeviceBuilder(const DeviceBuilder&) = delete;
  DeviceBuilder& operator=(const DeviceBuilder&) = delete;

  DeviceBuilder& setExtensions(const std::vector<std::string>& extensions);
  DeviceBuilder& setPhysicalDeviceFeatures(
      vk::PhysicalDeviceFeatures const* physicalDeviceFeatures);
  DeviceBuilder& setPNext(void const* pNext);
  vk::Device build();

 private:
  const vk::PhysicalDevice& _physicalDevice;
  uint32_t _queueFamilyIndex;
  std::vector<std::string> _extensions;
  vk::PhysicalDeviceFeatures const* _physicalDeviceFeatures;
  void const* _pNext;
};
}  // namespace engine
