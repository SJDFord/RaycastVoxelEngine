#include "descriptors.hpp"

// std
#include <cassert>
#include <stdexcept>

// *************** Descriptor Set Layout Builder *********************

DescriptorSetLayout::Builder &DescriptorSetLayout::Builder::addBinding(
    uint32_t binding,
    vk::DescriptorType descriptorType,
    vk::ShaderStageFlags stageFlags,
    uint32_t count) {
  assert(bindings.count(binding) == 0 && "Binding already in use");
  vk::DescriptorSetLayoutBinding layoutBinding = vk::DescriptorSetLayoutBinding()
      .setBinding(binding)
      .setDescriptorType(descriptorType)
      .setDescriptorCount(count)
      .setStageFlags(stageFlags);
  bindings[binding] = layoutBinding;
  return *this;
}

std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::Builder::build() const {
  return std::make_unique<DescriptorSetLayout>(device, bindings);
}

// *************** Descriptor Set Layout *********************

DescriptorSetLayout::DescriptorSetLayout(
    Device &device, std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings)
    : device{device}, bindings{bindings} {
  std::vector<vk::DescriptorSetLayoutBinding> setLayoutBindings{};
  for (const std::pair <uint32_t, vk::DescriptorSetLayoutBinding>& kv : bindings) {
    setLayoutBindings.push_back(kv.second);
  }

  vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutInfo = vk::DescriptorSetLayoutCreateInfo()
      .setBindingCount(static_cast<uint32_t>(setLayoutBindings.size()))
      .setBindings(setLayoutBindings);

  descriptorSetLayout = device.device().createDescriptorSetLayout(descriptorSetLayoutInfo);
}

DescriptorSetLayout::~DescriptorSetLayout() {
    // TODO:
    //vkDestroyDescriptorSetLayout(lveDevice.device(), descriptorSetLayout, nullptr);
}

// *************** Descriptor Pool Builder *********************

DescriptorPool::Builder &DescriptorPool::Builder::addPoolSize(
    vk::DescriptorType descriptorType, uint32_t count) {
  poolSizes.push_back({descriptorType, count});
  return *this;
}

DescriptorPool::Builder &DescriptorPool::Builder::setPoolFlags(
    vk::DescriptorPoolCreateFlags flags) {
  poolFlags = flags;
  return *this;
}
DescriptorPool::Builder &DescriptorPool::Builder::setMaxSets(uint32_t count) {
  maxSets = count;
  return *this;
}

std::unique_ptr<DescriptorPool> DescriptorPool::Builder::build() const {
  return std::make_unique<DescriptorPool>(device, maxSets, poolFlags, poolSizes);
}

// *************** Descriptor Pool *********************

DescriptorPool::DescriptorPool(
    Device &Device,
    uint32_t maxSets,
    vk::DescriptorPoolCreateFlags poolFlags,
    const std::vector<vk::DescriptorPoolSize> &poolSizes)
    : device{device} {
  VkDescriptorPoolCreateInfo descriptorPoolInfo = vk::DescriptorPoolCreateInfo()
      .setPoolSizeCount(static_cast<uint32_t>(poolSizes.size()))
      .setPoolSizes(poolSizes)
      .setMaxSets(maxSets)
      .setFlags(poolFlags);
  
  descriptorPool = device.device().createDescriptorPool(descriptorPoolInfo);
}

DescriptorPool::~DescriptorPool() {
  //vkDestroyDescriptorPool(lveDevice.device(), descriptorPool, nullptr);
}

bool DescriptorPool::allocateDescriptor(
    const vk::raii::DescriptorSetLayout& descriptorSetLayout,
    vk::raii::DescriptorSet &descriptor) const {
  
  vk::DescriptorSetAllocateInfo allocInfo = vk::DescriptorSetAllocateInfo()
      .setDescriptorPool(descriptorPool)
      .setSetLayouts(*descriptorSetLayout)
      .setDescriptorSetCount(1);

  auto descriptors = device.device().allocateDescriptorSets(allocInfo);
  if (descriptors.size() > 0) {
    descriptor = std::move(descriptors[0]);
    return true;
  }

  return false;

  // Might want to create a "DescriptorPoolManager" class that handles this case, and builds
  // a new pool whenever an old pool fills up. But this is beyond our current scope
  /*
  if (vkAllocateDescriptorSets(lveDevice.device(), &allocInfo, &descriptor) != VK_SUCCESS) {
    return false;
  }
  return true;
  */
}

void DescriptorPool::freeDescriptors(std::vector<vk::raii::DescriptorSet> &descriptors) const {
    // TODO    
    /*
    vkFreeDescriptorSets(
      lveDevice.device(),
      descriptorPool,
      static_cast<uint32_t>(descriptors.size()),
      descriptors.data());
    */
}

void DescriptorPool::resetPool() {
    descriptorPool.reset();
}

// *************** Descriptor Writer *********************

DescriptorWriter::DescriptorWriter(DescriptorSetLayout &setLayout, DescriptorPool &pool)
    : setLayout{setLayout}, pool{pool} {}

DescriptorWriter &DescriptorWriter::writeBuffer(
    uint32_t binding, vk::DescriptorBufferInfo *bufferInfo) {
  assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

  auto &bindingDescription = setLayout.bindings[binding];

  assert(
      bindingDescription.descriptorCount == 1 &&
      "Binding single descriptor info, but binding expects multiple");

  vk::WriteDescriptorSet write = vk::WriteDescriptorSet()
      .setDescriptorType(bindingDescription.descriptorType)
      .setDstBinding(binding)
      .setPBufferInfo(bufferInfo)
      .setDescriptorCount(1);
  writes.push_back(write);
  return *this;
}

DescriptorWriter &DescriptorWriter::writeImage(
    uint32_t binding, vk::DescriptorImageInfo *imageInfo) {
  assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

  auto &bindingDescription = setLayout.bindings[binding];

  assert(
      bindingDescription.descriptorCount == 1 &&
      "Binding single descriptor info, but binding expects multiple");

  vk::WriteDescriptorSet write = vk::WriteDescriptorSet()
      .setDescriptorType(bindingDescription.descriptorType)
      .setDstBinding(binding)
      .setPImageInfo(imageInfo)
      .setDescriptorCount(1);

  writes.push_back(write);
  return *this;
}

bool DescriptorWriter::build(vk::raii::DescriptorSet &set) {
  bool success = pool.allocateDescriptor(setLayout.getDescriptorSetLayout(), set);
  if (!success) {
    return false;
  }
  overwrite(set);
  return true;
}

void DescriptorWriter::overwrite(vk::raii::DescriptorSet &set) {
  for (auto &write : writes) {
    write.dstSet = set;
  }
  pool.device.device().updateDescriptorSets(writes, {});
}
