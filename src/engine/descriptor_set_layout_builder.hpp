#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_raii.hpp"

namespace engine {

class DescriptorSetLayoutBuilder {
 public:
  DescriptorSetLayoutBuilder(vk::Device const& device);
  ~DescriptorSetLayoutBuilder();

  DescriptorSetLayoutBuilder(const DescriptorSetLayoutBuilder&) = delete;
  DescriptorSetLayoutBuilder& operator=(const DescriptorSetLayoutBuilder&) = delete;

  DescriptorSetLayoutBuilder& addBinding(
      vk::DescriptorType descriptorType, uint32_t descriptorCount, vk::ShaderStageFlags stageFlags);
  DescriptorSetLayoutBuilder& setFlags(vk::DescriptorSetLayoutCreateFlags flags);
  vk::DescriptorSetLayout build();

 private:
  vk::Device _device;
  std::vector<std::tuple<vk::DescriptorType, uint32_t, vk::ShaderStageFlags>> _bindingData;
  vk::DescriptorSetLayoutCreateFlags _flags;
};

}  // namespace engine