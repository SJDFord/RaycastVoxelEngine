#pragma once

#include <vector>
#include <iostream>

#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_raii.hpp"

namespace engine {

class PhysicalDeviceStrategy {
 public:
      PhysicalDeviceStrategy(){}
    virtual ~PhysicalDeviceStrategy(){}
  //virtual ~PhysicalDeviceStrategy() {}
  virtual vk::PhysicalDevice pickPhysicalDevice(
      const std::vector<vk::PhysicalDevice>& physicalDevices) = 0;
};

}  // namespace engine
