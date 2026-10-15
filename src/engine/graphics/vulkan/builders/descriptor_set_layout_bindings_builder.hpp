#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "vulkan/vulkan.hpp"

namespace engine {

class DescriptorSetLayoutBindingsBuilder {
 public:
  DescriptorSetLayoutBindingsBuilder();
  ~DescriptorSetLayoutBindingsBuilder();

  DescriptorSetLayoutBindingsBuilder(const DescriptorSetLayoutBindingsBuilder&) = delete;
  DescriptorSetLayoutBindingsBuilder& operator=(const DescriptorSetLayoutBindingsBuilder&) = delete;

  DescriptorSetLayoutBindingsBuilder& addBinding(
    vk::DescriptorType descriptorType, 
    uint32_t descriptorCount, 
    vk::ShaderStageFlags stageFlags
  );
  std::vector<vk::DescriptorSetLayoutBinding> build();

 private:
  std::vector<std::tuple<vk::DescriptorType, uint32_t, vk::ShaderStageFlags>> _bindingData;
};

}  // namespace engine