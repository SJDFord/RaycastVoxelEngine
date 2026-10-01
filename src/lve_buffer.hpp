#pragma once

#include "lve_device.hpp"

namespace lve {

class LveBuffer {
 public:
  LveBuffer(
      LveDevice& device,
      vk::DeviceSize instanceSize,
      uint32_t instanceCount,
      vk::BufferUsageFlags usageFlags,
      vk::MemoryPropertyFlags memoryPropertyFlags,
      vk::DeviceSize minOffsetAlignment = 1);
  ~LveBuffer();

  LveBuffer(const LveBuffer&) = delete;
  LveBuffer& operator=(const LveBuffer&) = delete;

  VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
  void unmap();

  void writeToBuffer(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
  VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
  VkDescriptorBufferInfo descriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
  VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

  void writeToIndex(void* data, int index);
  VkResult flushIndex(int index);
  VkDescriptorBufferInfo descriptorInfoForIndex(int index);
  VkResult invalidateIndex(int index);

  vk::Buffer getBuffer() const { return buffer; }
  void* getMappedMemory() const { return mapped; }
  uint32_t getInstanceCount() const { return instanceCount; }
  vk::DeviceSize getInstanceSize() const { return instanceSize; }
  vk::DeviceSize getAlignmentSize() const { return instanceSize; }
  vk::BufferUsageFlags getUsageFlags() const { return usageFlags; }
  vk::MemoryPropertyFlags getMemoryPropertyFlags() const { return memoryPropertyFlags; }
  vk::DeviceSize getBufferSize() const { return bufferSize; }

 private:
  static VkDeviceSize getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment);

  LveDevice& lveDevice;
  void* mapped = nullptr;
  vk::Buffer buffer = VK_NULL_HANDLE;
  vk::DeviceMemory memory = VK_NULL_HANDLE;

  vk::DeviceSize bufferSize;
  uint32_t instanceCount;
  vk::DeviceSize instanceSize;
  vk::DeviceSize alignmentSize;
  vk::BufferUsageFlags usageFlags;
  vk::MemoryPropertyFlags memoryPropertyFlags;
};

}  // namespace lve
