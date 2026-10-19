#pragma once

#include "vulkan/vulkan.hpp"
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
  vk::Result map(VkDeviceSize size = vk::WholeSize, VkDeviceSize offset = 0);
  void unmap();

  void writeToBuffer(void* data, VkDeviceSize size = vk::WholeSize, VkDeviceSize offset = 0);
  vk::Result flush(VkDeviceSize size = vk::WholeSize, VkDeviceSize offset = 0);
  vk::DescriptorBufferInfo descriptorInfo(VkDeviceSize size = vk::WholeSize, VkDeviceSize offset = 0);
  vk::Result invalidate(VkDeviceSize size = vk::WholeSize, VkDeviceSize offset = 0);

  void writeToIndex(void* data, int index);
  vk::Result flushIndex(int index);
  vk::DescriptorBufferInfo descriptorInfoForIndex(int index);
  vk::Result invalidateIndex(int index);

  void copyTo(const vk::CommandBuffer &commandBuffer, const Buffer &destination);
  void copyTo(const vk::CommandBuffer &commandBuffer, const vk::Image &destination, vk::Extent2D extent);
  void bindVertex(const vk::CommandBuffer &commandBuffer, uint32_t firstBinding, std::vector<vk::DeviceSize> offsets);
  void bindIndex(const vk::CommandBuffer &commandBuffer, vk::DeviceSize offset, vk::IndexType indexType);

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
