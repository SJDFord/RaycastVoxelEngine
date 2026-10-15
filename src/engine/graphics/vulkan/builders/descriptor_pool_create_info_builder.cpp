
#include "descriptor_pool_create_info_builder.hpp"

namespace engine {

DescriptorPoolCreateInfoBuilder::DescriptorPoolCreateInfoBuilder() {
}

DescriptorPoolCreateInfoBuilder::~DescriptorPoolCreateInfoBuilder() {}

DescriptorPoolCreateInfoBuilder& DescriptorPoolCreateInfoBuilder::addPoolSize(vk::DescriptorType descriptorType, uint32_t count) {
  _poolSizes.push_back({ descriptorType, count });
  return *this;
}

DescriptorPoolCreateInfoBuilder& DescriptorPoolCreateInfoBuilder::setFlags(vk::DescriptorPoolCreateFlags flags) {
  _flags = flags;
  return *this;
}

DescriptorPoolCreateInfoBuilder& DescriptorPoolCreateInfoBuilder::setMaxSets(uint32_t count) {
  _maxSets = count;
  return *this;
}

vk::DescriptorPoolCreateInfo DescriptorPoolCreateInfoBuilder::build() {
  vk::DescriptorPoolCreateInfo createInfo = vk::DescriptorPoolCreateInfo()
    .setPoolSizes(_poolSizes)
    .setMaxSets(_maxSets)
    .setFlags(_flags);
  return createInfo;
}

}  // namespace engine