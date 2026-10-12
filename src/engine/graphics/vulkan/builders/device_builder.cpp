#include "device_builder.hpp"

namespace engine {

DeviceBuilder::DeviceBuilder(const vk::PhysicalDevice& physicalDevice, uint32_t queueFamilyIndex):

    _physicalDevice(physicalDevice),
    _queueFamilyIndex(queueFamilyIndex),
    _physicalDeviceFeatures(nullptr),
    _pNext(nullptr){
}

DeviceBuilder::~DeviceBuilder() {}

DeviceBuilder& DeviceBuilder::setExtensions(const std::vector<std::string>& extensions) {
  _extensions = extensions;
  return *this;
}

DeviceBuilder& DeviceBuilder::setPhysicalDeviceFeatures(
    vk::PhysicalDeviceFeatures const* physicalDeviceFeatures) {
  _physicalDeviceFeatures = physicalDeviceFeatures;
  return *this;
}

DeviceBuilder& DeviceBuilder::setPNext(void const* pNext) {
  _pNext = pNext;
  return *this;
}

vk::Device DeviceBuilder::build() {
  std::vector<char const*> enabledExtensions;
  enabledExtensions.reserve(_extensions.size());
  for (auto const& ext : _extensions) {
    enabledExtensions.push_back(ext.data());
  }

  float queuePriority = 0.0f;
  vk::DeviceQueueCreateInfo deviceQueueCreateInfo({}, _queueFamilyIndex, 1, &queuePriority);
  vk::DeviceCreateInfo deviceCreateInfo(
      {},
      deviceQueueCreateInfo,
      {},
      enabledExtensions,
      _physicalDeviceFeatures,
      _pNext);

  vk::Device device = _physicalDevice.createDevice(deviceCreateInfo);

#if (VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1)
  // initialize function pointers for instance
  VULKAN_HPP_DEFAULT_DISPATCHER.init(device);
#endif
  return device;
}
}  // namespace engine

