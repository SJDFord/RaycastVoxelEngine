#include "descriptors.hpp"

// std
#include <cassert>
#include <stdexcept>

namespace engine {

// *************** Descriptor Writer *********************

DescriptorWriter::DescriptorWriter(vk::Device device, vk::DescriptorPool pool, vk::DescriptorSetLayout setLayout, std::vector<vk::DescriptorSetLayoutBinding> bindings)
    : _device{device}, _pool{pool}, _setLayout{setLayout}, _bindings{bindings}, _writes{} {}

DescriptorWriter &DescriptorWriter::writeBuffer(
    uint32_t binding, vk::DescriptorBufferInfo* bufferInfo) {
  assert(_bindings.size() > binding && "Layout does not contain specified binding");

  auto &bindingDescription = _bindings[binding];

  assert(
      bindingDescription.descriptorCount == 1 &&
      "Binding single descriptor info, but binding expects multiple");

  vk::WriteDescriptorSet write = vk::WriteDescriptorSet()
    .setDescriptorType(bindingDescription.descriptorType)
    .setDstBinding(binding)
    .setPBufferInfo(bufferInfo)
    .setDescriptorCount(1);

  _writes.push_back(write);

  return *this;
}

DescriptorWriter &DescriptorWriter::writeImage(
    uint32_t binding, vk::DescriptorImageInfo* imageInfo) {
  assert(_bindings.size() > binding && "Layout does not contain specified binding");

  auto &bindingDescription = _bindings[binding];

  assert(
      bindingDescription.descriptorCount == 1 &&
      "Binding single descriptor info, but binding expects multiple");


  vk::WriteDescriptorSet write = vk::WriteDescriptorSet()
    .setDescriptorType(bindingDescription.descriptorType)
    .setDstBinding(binding)
    .setPImageInfo(imageInfo)
    .setDescriptorCount(1);

  _writes.push_back(write);
  return *this;
}

/*
  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = descriptorPool;
  allocInfo.pSetLayouts = &descriptorSetLayout;
  allocInfo.descriptorSetCount = 1;

  // Might want to create a "DescriptorPoolManager" class that handles this case, and builds
  // a new pool whenever an old pool fills up. But this is beyond our current scope
  if (vkAllocateDescriptorSets(lveDevice.device(), &allocInfo, &descriptor) != VK_SUCCESS) {
    return false;
  }
  return true;
*/

vk::DescriptorSet DescriptorWriter::build() {
  vk::DescriptorSetAllocateInfo allocateInfo = vk::DescriptorSetAllocateInfo()
    .setDescriptorPool(_pool)
    .setSetLayouts(_setLayout)
    .setDescriptorSetCount(1);
  std::vector<vk::DescriptorSet> sets = _device.allocateDescriptorSets(allocateInfo);
  auto set = sets.front();
  overwrite(set);
  return set;
}

void DescriptorWriter::overwrite(vk::DescriptorSet &set) {
  for (auto &write : _writes) {
    write.dstSet = set;
  }
  _device.updateDescriptorSets(_writes, {});
}

}  // namespace lve
