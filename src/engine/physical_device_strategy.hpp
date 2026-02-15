#pragma once

#include <vector>

#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_raii.hpp"

namespace engine {

class PhysicalDeviceStrategy {
 public:
  //virtual ~PhysicalDeviceStrategy() {}
  virtual vk::PhysicalDevice pickPhysicalDevice(
      const std::vector<vk::PhysicalDevice>& physicalDevices);
};

}  // namespace engine
