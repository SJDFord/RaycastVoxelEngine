#pragma once

#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <functional>

namespace engine {

class OneTimeCommandSubmitter {
 public:
  OneTimeCommandSubmitter(vk::Device device, vk::CommandPool commandPool, uint32_t graphicsFamilyIndex);
  ~OneTimeCommandSubmitter();

  OneTimeCommandSubmitter(const OneTimeCommandSubmitter&) = delete;
  OneTimeCommandSubmitter& operator=(const OneTimeCommandSubmitter&) = delete;

  void submit(std::function<void(vk::CommandBuffer const &)> commandBufferOperation);
 private:
  vk::Device _device;
  vk::CommandPool _commandPool;
  uint32_t _graphicsFamilyIndex;
};


}