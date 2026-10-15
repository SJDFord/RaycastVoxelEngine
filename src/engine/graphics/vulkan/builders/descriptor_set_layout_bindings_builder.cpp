#include "descriptor_set_layout_bindings_builder.hpp"

namespace engine {

DescriptorSetLayoutBindingsBuilder::DescriptorSetLayoutBindingsBuilder() {}

DescriptorSetLayoutBindingsBuilder::~DescriptorSetLayoutBindingsBuilder() {}

DescriptorSetLayoutBindingsBuilder& DescriptorSetLayoutBindingsBuilder::addBinding(
    vk::DescriptorType descriptorType, 
    uint32_t descriptorCount, 
    vk::ShaderStageFlags stageFlags
) {
  _bindingData.push_back({descriptorType, descriptorCount, stageFlags});
  return *this;
}

std::vector<vk::DescriptorSetLayoutBinding> DescriptorSetLayoutBindingsBuilder::build() {
  std::vector<vk::DescriptorSetLayoutBinding> bindings;
  for (size_t i = 0; i < _bindingData.size(); i++) {
    bindings.emplace_back(
        static_cast<uint32_t>(i),
        std::get<0>(_bindingData[i]),
        std::get<1>(_bindingData[i]),
        std::get<2>(_bindingData[i])
    );
  }
  return bindings;
}

}  // namespace engine

