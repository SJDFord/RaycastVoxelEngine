#pragma once

// std
#include <memory>
#include <unordered_map>
#include <vector>

#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_raii.hpp"

namespace engine {

class DescriptorWriter {
 public:
  DescriptorWriter(vk::Device device, vk::DescriptorPool pool, vk::DescriptorSetLayout setLayout, std::vector<vk::DescriptorSetLayoutBinding> bindings);

  DescriptorWriter &writeBuffer(uint32_t binding, vk::DescriptorBufferInfo* bufferInfo);
  DescriptorWriter &writeImage(uint32_t binding, vk::DescriptorImageInfo* imageInfo);

  vk::DescriptorSet build();
  void overwrite(vk::DescriptorSet &set);

 private:
  vk::Device _device;
  vk::DescriptorPool _pool;
  vk::DescriptorSetLayout _setLayout;
  std::vector<vk::DescriptorSetLayoutBinding> _bindings;
  std::vector<vk::WriteDescriptorSet> _writes;
};

}  // namespace lve
