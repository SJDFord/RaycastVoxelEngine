#pragma once

#include "physical_device_strategy.hpp"

#include <iostream>

namespace engine {

class RankedPhysicalDeviceStrategy : public PhysicalDeviceStrategy {
 public:
  vk::PhysicalDevice pickPhysicalDevice(
      const std::vector<vk::PhysicalDevice>& physicalDevices) override;
 private:
        int32_t scoreDevice(const vk::PhysicalDevice& physicalDevice);
        //vk::PhysicalDevice getBestAvailablePhysicalDevice();
        //void getAttributes();
        //void retrieveQueueFamilyProperties();
};

}  // namespace engine