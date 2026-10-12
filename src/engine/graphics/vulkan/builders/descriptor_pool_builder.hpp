#pragma once

#include <vector>

#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_raii.hpp"

namespace engine {

class DescriptorPoolBuilder {
 public:
  DescriptorPoolBuilder(vk::Device const& device);
  ~DescriptorPoolBuilder();

  DescriptorPoolBuilder(const DescriptorPoolBuilder&) = delete;
  DescriptorPoolBuilder& operator=(const DescriptorPoolBuilder&) = delete;

  DescriptorPoolBuilder& addPoolSize(vk::DescriptorType descriptorType, uint32_t count);
  DescriptorPoolBuilder& setFlags(vk::DescriptorPoolCreateFlags flags);
  DescriptorPoolBuilder& setMaxSets(uint32_t count);
  vk::DescriptorPool build();

 private:
    vk::Device _device;
    std::vector<vk::DescriptorPoolSize> _poolSizes;
    vk::DescriptorPoolCreateFlags _flags;
    uint32_t _maxSets;
};

}  // namespace engine