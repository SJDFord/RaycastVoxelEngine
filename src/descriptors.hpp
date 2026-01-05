#pragma once

#include "device.hpp"

// std
#include <memory>
#include <unordered_map>
#include <vector>

class DescriptorSetLayout {
 public:
  class Builder {
   public:
    Builder(Device &device) : device{device} {}

    Builder &addBinding(
        uint32_t binding,
        vk::DescriptorType descriptorType,
        vk::ShaderStageFlags stageFlags,
        uint32_t count = 1);
    std::unique_ptr<DescriptorSetLayout> build() const;

   private:
    Device &device;
    std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings{};
  };

  DescriptorSetLayout(
      Device &device, std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings);
  ~DescriptorSetLayout();
  DescriptorSetLayout(const DescriptorSetLayout &) = delete;
  DescriptorSetLayout &operator=(const DescriptorSetLayout &) = delete;

  const vk::raii::DescriptorSetLayout& getDescriptorSetLayout() const { return descriptorSetLayout; }

 private:
  Device &device;
  vk::raii::DescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
  std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings;

  friend class DescriptorWriter;
};

class DescriptorPool {
 public:
  class Builder {
   public:
    Builder(std::shared_ptr<Device> device) : device{device} {}

    Builder &addPoolSize(vk::DescriptorType descriptorType, uint32_t count);
    Builder &setPoolFlags(vk::DescriptorPoolCreateFlags flags);
    Builder &setMaxSets(uint32_t count);
    std::unique_ptr<DescriptorPool> build() const;

   private:
    std::shared_ptr<Device> device;
    std::vector<vk::DescriptorPoolSize> poolSizes{};
    uint32_t maxSets = 1000;
    vk::DescriptorPoolCreateFlags poolFlags;
  };

  DescriptorPool(
      std::shared_ptr<Device> device,
      uint32_t maxSets,
      vk::DescriptorPoolCreateFlags poolFlags,
      const std::vector<vk::DescriptorPoolSize> &poolSizes);
  ~DescriptorPool();
  DescriptorPool(const DescriptorPool &) = delete;
  DescriptorPool &operator=(const DescriptorPool &) = delete;

  bool allocateDescriptor(
      const vk::raii::DescriptorSetLayout& descriptorSetLayout,
      vk::raii::DescriptorSet &descriptor) const;

  void freeDescriptors(std::vector<vk::raii::DescriptorSet> &descriptors) const;

  void resetPool();

 private:
  std::shared_ptr<Device> device;
  vk::raii::DescriptorPool descriptorPool = VK_NULL_HANDLE;

  friend class DescriptorWriter;
};

class DescriptorWriter {
 public:
  DescriptorWriter(DescriptorSetLayout &setLayout, DescriptorPool &pool);

  DescriptorWriter &writeBuffer(uint32_t binding, vk::DescriptorBufferInfo *bufferInfo);
  DescriptorWriter &writeImage(uint32_t binding, vk::DescriptorImageInfo *imageInfo);

  bool build(vk::raii::DescriptorSet &set);
  void overwrite(vk::raii::DescriptorSet &set);

 private:
  DescriptorSetLayout &setLayout;
  DescriptorPool &pool;
  std::vector<vk::WriteDescriptorSet> writes;
};