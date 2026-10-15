#pragma once

#include <vector>
#include <cassert>

#include "vulkan/vulkan.hpp"

namespace engine {

class DescriptorPoolCreateInfoBuilder {
 public:
  DescriptorPoolCreateInfoBuilder();
  ~DescriptorPoolCreateInfoBuilder();

  DescriptorPoolCreateInfoBuilder(const DescriptorPoolCreateInfoBuilder&) = delete;
  DescriptorPoolCreateInfoBuilder& operator=(const DescriptorPoolCreateInfoBuilder&) = delete;

  DescriptorPoolCreateInfoBuilder& addPoolSize(vk::DescriptorType descriptorType, uint32_t count);
  DescriptorPoolCreateInfoBuilder& setFlags(vk::DescriptorPoolCreateFlags flags);
  DescriptorPoolCreateInfoBuilder& setMaxSets(uint32_t count);
  vk::DescriptorPoolCreateInfo build();

 private:
    std::vector<vk::DescriptorPoolSize> _poolSizes;
    vk::DescriptorPoolCreateFlags _flags;
    uint32_t _maxSets;
};

}  // namespace engine