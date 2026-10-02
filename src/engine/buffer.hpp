#pragma once

#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_raii.hpp"
#include "utils.hpp"

namespace engine {

class Buffer {
 public:
  Buffer(
      vk::Device device,
      vk::PhysicalDevice physicalDevice,
      vk::DeviceSize instanceSize,
      uint32_t instanceCount,
      vk::BufferUsageFlags usageFlags,
      vk::MemoryPropertyFlags memoryPropertyFlags,
      vk::DeviceSize minOffsetAlignment = 1);
  ~Buffer();

  Buffer(const Buffer&) = delete;
  Buffer& operator=(const Buffer&) = delete;

  vk::Result map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
  void unmap();

  void writeToBuffer(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
  vk::Result flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
  vk::DescriptorBufferInfo descriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
  vk::Result invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

  void writeToIndex(void* data, int index);
  vk::Result flushIndex(int index);
  vk::DescriptorBufferInfo descriptorInfoForIndex(int index);
  vk::Result invalidateIndex(int index);

  vk::Buffer getBuffer() const { return _buffer; }
  void* getMappedMemory() const { return _mapped; }
  uint32_t getInstanceCount() const { return _instanceCount; }
  vk::DeviceSize getInstanceSize() const { return _instanceSize; }
  vk::DeviceSize getAlignmentSize() const { return _instanceSize; }
  vk::BufferUsageFlags getUsageFlags() const { return _usageFlags; }
  vk::MemoryPropertyFlags getMemoryPropertyFlags() const { return _memoryPropertyFlags; }
  vk::DeviceSize getBufferSize() const { return _bufferSize; }

 private:
  static vk::DeviceSize getAlignment(vk::DeviceSize instanceSize, vk::DeviceSize minOffsetAlignment);

  vk::Device _device;
  vk::PhysicalDevice _physicalDevice;

  void* _mapped = nullptr;
  vk::Buffer _buffer = VK_NULL_HANDLE;
  vk::DeviceMemory _memory = VK_NULL_HANDLE;

  vk::DeviceSize _bufferSize;
  uint32_t _instanceCount;
  vk::DeviceSize _instanceSize;
  vk::DeviceSize _alignmentSize;
  vk::BufferUsageFlags _usageFlags;
  vk::MemoryPropertyFlags _memoryPropertyFlags;
};

}
