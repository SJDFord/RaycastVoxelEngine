#pragma once

#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_raii.hpp"
#include <device.hpp>


class BufferRaii {
 public:
  BufferRaii(
      const vk::PhysicalDevice& physicalDevice,
      const vk::Device& device, 
      vk::DeviceSize instanceSize,
      uint32_t instanceCount,
      vk::BufferUsageFlags usageFlags,
      vk::MemoryPropertyFlags memoryPropertyFlags,
      vk::DeviceSize minOffsetAlignment = 1);
  ~BufferRaii();

  BufferRaii(const BufferRaii&) = delete;
  BufferRaii& operator=(const BufferRaii&) = delete;

  /*
  void map(vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0);
  void unmap();
  */

  void writeToBuffer(void* data, vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0);
  void flush(vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0);
  vk::DescriptorBufferInfo descriptorInfo(vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0);
  void invalidate(vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0);

  void writeToIndex(void* data, int index);
  void flushIndex(int index);
  vk::DescriptorBufferInfo descriptorInfoForIndex(int index);
  void invalidateIndex(int index);
  

  const vk::Buffer& getBuffer() const { return _buffer; }
  void* getMappedMemory() const { return _mapped; }
  uint32_t getInstanceCount() const { return instanceCount; }
  vk::DeviceSize getInstanceSize() const { return instanceSize; }
  vk::DeviceSize getAlignmentSize() const { return instanceSize; }
  vk::BufferUsageFlags getUsageFlags() const { return usageFlags; }
  vk::MemoryPropertyFlags getMemoryPropertyFlags() const { return memoryPropertyFlags; }
  vk::DeviceSize getBufferSize() const { return bufferSize; }

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
  vk::DeviceSize bufferSize;
  uint32_t instanceCount;
  vk::DeviceSize instanceSize;
  vk::DeviceSize alignmentSize;
  vk::BufferUsageFlags usageFlags;
  vk::MemoryPropertyFlags memoryPropertyFlags;
  
  vk::Buffer _buffer;
  vk::DeviceMemory _memory;
};
