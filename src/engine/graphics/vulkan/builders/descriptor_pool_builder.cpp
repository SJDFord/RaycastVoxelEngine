
#include "descriptor_pool_builder.hpp"

namespace engine {

DescriptorPoolBuilder::DescriptorPoolBuilder(vk::Device const& device) {
  _device = device;
}

DescriptorPoolBuilder::~DescriptorPoolBuilder() {}



DescriptorPoolBuilder& DescriptorPoolBuilder::addPoolSize(vk::DescriptorType descriptorType, uint32_t count) {
    _poolSizes.push_back({ descriptorType, count });
    return *this;
}

DescriptorPoolBuilder& DescriptorPoolBuilder::setFlags(vk::DescriptorPoolCreateFlags flags) {
  _flags = flags;
  return *this;
}

DescriptorPoolBuilder& DescriptorPoolBuilder::setMaxSets(uint32_t count) {
  _maxSets = count;
  return *this;
}

vk::DescriptorPool DescriptorPoolBuilder::build() {
  vk::DescriptorPoolCreateInfo createInfo = vk::DescriptorPoolCreateInfo()
    .setPoolSizes(_poolSizes)
    .setMaxSets(_maxSets)
    .setFlags(_flags);

  return _device.createDescriptorPool(createInfo);
}

}  // namespace engine