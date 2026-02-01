#pragma once

#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_raii.hpp"
#include <device.hpp>


class BufferRaii {
 public:
  BufferRaii(
        const vk::PhysicalDevice& physicalDevice,      
    const vk::Device& device,
    vk::DeviceSize size,
    vk::BufferUsageFlags usage,
    vk::MemoryPropertyFlags propertyFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
  ~BufferRaii();

  BufferRaii(const BufferRaii&) = delete;
  BufferRaii& operator=(const BufferRaii&) = delete;

  /*
  void map(vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0);
  void unmap();
  */

  //void writeToBuffer(void* data, vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0);
  void flush(vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0);
  vk::DescriptorBufferInfo descriptorInfo(vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0);
  void invalidate(vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0);

  void clear();
  //void writeToIndex(void* data, int index);
  //void flushIndex(int index);
  //vk::DescriptorBufferInfo descriptorInfoForIndex(int index);
  //void invalidateIndex(int index);
  

  const vk::Buffer& getBuffer() const { return _buffer; }
  const vk::DeviceMemory& getDeviceMemory() const { return _memory; }
  void* getMappedMemory() const { return _mapped; }
  vk::BufferUsageFlags getUsageFlags() const { return usage; }
  vk::MemoryPropertyFlags getMemoryPropertyFlags() const { return propertyFlags; }
 private:
  static vk::DeviceSize getAlignment(vk::DeviceSize instanceSize, vk::DeviceSize minOffsetAlignment);

        uint32_t findMemoryType(
                vk::PhysicalDeviceMemoryProperties const & memoryProperties, 
                uint32_t typeBits, 
                vk::MemoryPropertyFlags requirementsMask 
        );

  void* _mapped = nullptr;
  //vk::raii::Buffer* buffer = VK_NULL_HANDLE;
  //vk::raii::DeviceMemory* memory = VK_NULL_HANDLE;

  const vk::Device& _device;
  vk::DeviceSize size;
  vk::BufferUsageFlags usage;
  vk::MemoryPropertyFlags propertyFlags;
  
  vk::Buffer _buffer;
  vk::DeviceMemory _memory;
};
