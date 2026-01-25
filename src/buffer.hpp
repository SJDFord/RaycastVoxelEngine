#pragma once

#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_raii.hpp"
#include <device.hpp>


class Buffer {
 public:
  Buffer(
      std::shared_ptr<Device> device,
      vk::DeviceSize instanceSize,
      uint32_t instanceCount,
      vk::BufferUsageFlags usageFlags,
      vk::MemoryPropertyFlags memoryPropertyFlags,
      vk::DeviceSize minOffsetAlignment = 1);
  ~Buffer();

  Buffer(const Buffer&) = delete;
  Buffer& operator=(const Buffer&) = delete;

  void map(vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0);
  void unmap();

  void writeToBuffer(void* data, vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0);
  void flush(vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0);
  vk::DescriptorBufferInfo descriptorInfo(vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0);
  void invalidate(vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0);

  void writeToIndex(void* data, int index);
  void flushIndex(int index);
  vk::DescriptorBufferInfo descriptorInfoForIndex(int index);
  void invalidateIndex(int index);

  const vk::raii::Buffer& getBuffer() const { return _buffer; }
  void* getMappedMemory() const { return _mapped; }
  uint32_t getInstanceCount() const { return instanceCount; }
  vk::DeviceSize getInstanceSize() const { return instanceSize; }
  vk::DeviceSize getAlignmentSize() const { return instanceSize; }
  vk::BufferUsageFlags getUsageFlags() const { return usageFlags; }
  vk::MemoryPropertyFlags getMemoryPropertyFlags() const { return memoryPropertyFlags; }
  vk::DeviceSize getBufferSize() const { return bufferSize; }

 private:
  static vk::DeviceSize getAlignment(vk::DeviceSize instanceSize, vk::DeviceSize minOffsetAlignment);

  void* _mapped = nullptr;
  //vk::raii::Buffer* buffer = VK_NULL_HANDLE;
  //vk::raii::DeviceMemory* memory = VK_NULL_HANDLE;

  std::shared_ptr<Device> _device;
  vk::DeviceSize bufferSize;
  uint32_t instanceCount;
  vk::DeviceSize instanceSize;
  vk::DeviceSize alignmentSize;
  vk::BufferUsageFlags usageFlags;
  vk::MemoryPropertyFlags memoryPropertyFlags;

  vk::raii::DeviceMemory _memory = VK_NULL_HANDLE;
  vk::raii::Buffer _buffer = VK_NULL_HANDLE;

};
