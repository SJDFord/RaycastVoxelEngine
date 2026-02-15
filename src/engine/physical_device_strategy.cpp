#include "physical_device_strategy.hpp"

namespace engine {
vk::PhysicalDevice PhysicalDeviceStrategy::pickPhysicalDevice(
    const std::vector<vk::PhysicalDevice>& physicalDevices) {
  return physicalDevices.front();
}
}  // namespace engine

