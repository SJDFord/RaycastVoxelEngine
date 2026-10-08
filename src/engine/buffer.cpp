#include "buffer.hpp"

/*
 * Encapsulates a vulkan buffer
 *
 * Initially based off VulkanBuffer by Sascha Willems -
 * https://github.com/SaschaWillems/Vulkan/blob/master/base/VulkanBuffer.h
 */

// std
#include <cassert>
#include <cstring>

namespace engine {

/**
 * Returns the minimum instance size required to be compatible with devices minOffsetAlignment
 *
 * @param instanceSize The size of an instance
 * @param minOffsetAlignment The minimum required alignment, in bytes, for the offset member (eg
 * minUniformBufferOffsetAlignment)
 *
 * @return VkResult of the buffer mapping call
 */
vk::DeviceSize Buffer::getAlignment(vk::DeviceSize instanceSize, vk::DeviceSize minOffsetAlignment) {
  if (minOffsetAlignment > 0) {
    return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
  }
  return instanceSize;
}

Buffer::Buffer(
    vk::Device device,
    vk::PhysicalDevice physicalDevice,
    vk::DeviceSize instanceSize,
    uint32_t instanceCount,
    vk::BufferUsageFlags usageFlags,
    vk::MemoryPropertyFlags memoryPropertyFlags,
    vk::DeviceSize minOffsetAlignment)
    : _device{device},
      _physicalDevice{physicalDevice},
      _instanceSize{instanceSize},
      _instanceCount{instanceCount},
      _usageFlags{usageFlags},
      _memoryPropertyFlags{memoryPropertyFlags} {
  _alignmentSize = getAlignment(instanceSize, minOffsetAlignment);
  _bufferSize = _alignmentSize * instanceCount;
  //device.createBuffer(_bufferSize, usageFlags, memoryPropertyFlags, _buffer, _memory);

  vk::BufferCreateInfo bufferInfo = vk::BufferCreateInfo()
    .setSize(_bufferSize)
    .setUsage(_usageFlags)
    .setSharingMode(vk::SharingMode::eExclusive);
  _buffer = device.createBuffer(bufferInfo);

  vk::DeviceBufferMemoryRequirements memoryRequirementsInfo =
    vk::DeviceBufferMemoryRequirements().setPCreateInfo(&bufferInfo);
  vk::MemoryRequirements memoryRequirements = device.getBufferMemoryRequirements(_buffer);

  vk::PhysicalDeviceMemoryProperties memoryProperties = physicalDevice.getMemoryProperties();
  uint32_t memoryTypeIndex = findMemoryType( memoryProperties, memoryRequirements.memoryTypeBits, _memoryPropertyFlags );

  vk::MemoryAllocateInfo allocInfo =
    vk::MemoryAllocateInfo()
        .setAllocationSize(memoryRequirements.size)
        .setMemoryTypeIndex(memoryTypeIndex);
  _memory = _device.allocateMemory(allocInfo);
  _device.bindBufferMemory(_buffer, _memory, 0);
}

Buffer::~Buffer() {
  unmap();
  _device.destroyBuffer(_buffer);
  _device.freeMemory(_memory);
}

/**
 * Map a memory range of this buffer. If successful, mapped points to the specified buffer range.
 *
 * @param size (Optional) Size of the memory range to map. Pass VK_WHOLE_SIZE to map the complete
 * buffer range.
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkResult of the buffer mapping call
 */
vk::Result Buffer::map(vk::DeviceSize size, vk::DeviceSize offset) {
  assert(_buffer && _memory && "Called map on buffer before create");
  return _device.mapMemory(_memory, offset, size, {}, &_mapped);
}

/**
 * Unmap a mapped memory range
 *
 * @note Does not return a result as vkUnmapMemory can't fail
 */
void Buffer::unmap() {
  if (_mapped) {
    _device.unmapMemory(_memory);
    _mapped = nullptr;
  }
}

/**
 * Copies the specified data to the mapped buffer. Default value writes whole buffer range
 *
 * @param data Pointer to the data to copy
 * @param size (Optional) Size of the data to copy. Pass VK_WHOLE_SIZE to flush the complete buffer
 * range.
 * @param offset (Optional) Byte offset from beginning of mapped region
 *
 */
void Buffer::writeToBuffer(void *data, vk::DeviceSize size, vk::DeviceSize offset) {
  assert(_mapped && "Cannot copy to unmapped buffer");

  if (size == VK_WHOLE_SIZE) {
    memcpy(_mapped, data, _bufferSize);
  } else {
    char *memOffset = (char *)_mapped;
    memOffset += offset;
    memcpy(memOffset, data, size);
  }
}

/**
 * Flush a memory range of the buffer to make it visible to the device
 *
 * @note Only required for non-coherent memory
 *
 * @param size (Optional) Size of the memory range to flush. Pass VK_WHOLE_SIZE to flush the
 * complete buffer range.
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkResult of the flush call
 */
vk::Result Buffer::flush(vk::DeviceSize size, vk::DeviceSize offset) {
  vk::MappedMemoryRange mappedRange = vk::MappedMemoryRange()
    .setMemory(_memory)
    .setOffset(offset)
    .setSize(size);
  _device.flushMappedMemoryRanges(mappedRange);
  return vk::Result::eSuccess;
}

/**
 * Invalidate a memory range of the buffer to make it visible to the host
 *
 * @note Only required for non-coherent memory
 *
 * @param size (Optional) Size of the memory range to invalidate. Pass VK_WHOLE_SIZE to invalidate
 * the complete buffer range.
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkResult of the invalidate call
 */
vk::Result Buffer::invalidate(vk::DeviceSize size, vk::DeviceSize offset) {
  vk::MappedMemoryRange mappedRange = vk::MappedMemoryRange()
    .setMemory(_memory)
    .setOffset(offset)
    .setSize(size);
  _device.invalidateMappedMemoryRanges(mappedRange);
  return vk::Result::eSuccess;
}

/**
 * Create a buffer info descriptor
 *
 * @param size (Optional) Size of the memory range of the descriptor
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkDescriptorBufferInfo of specified offset and range
 */
vk::DescriptorBufferInfo Buffer::descriptorInfo(vk::DeviceSize size, vk::DeviceSize offset) {
  vk::DescriptorBufferInfo info = vk::DescriptorBufferInfo()
    .setBuffer(_buffer)
    .setOffset(offset)
    .setRange(size);

  return info;
}

/**
 * Copies "instanceSize" bytes of data to the mapped buffer at an offset of index * alignmentSize
 *
 * @param data Pointer to the data to copy
 * @param index Used in offset calculation
 *
 */
void Buffer::writeToIndex(void *data, int index) {
  writeToBuffer(data, _instanceSize, index * _alignmentSize);
}

/**
 *  Flush the memory range at index * alignmentSize of the buffer to make it visible to the device
 *
 * @param index Used in offset calculation
 *
 */
vk::Result Buffer::flushIndex(int index) { return flush(_alignmentSize, index * _alignmentSize); }

/**
 * Create a buffer info descriptor
 *
 * @param index Specifies the region given by index * alignmentSize
 *
 * @return VkDescriptorBufferInfo for instance at index
 */
vk::DescriptorBufferInfo Buffer::descriptorInfoForIndex(int index) {
  return descriptorInfo(_alignmentSize, index * _alignmentSize);
}

/**
 * Invalidate a memory range of the buffer to make it visible to the host
 *
 * @note Only required for non-coherent memory
 *
 * @param index Specifies the region to invalidate: index * alignmentSize
 *
 * @return VkResult of the invalidate call
 */
vk::Result Buffer::invalidateIndex(int index) {
  return invalidate(_alignmentSize, index * _alignmentSize);
}

uint32_t Buffer::findMemoryType(
    vk::PhysicalDeviceMemoryProperties const& memoryProperties,
    uint32_t typeBits,
    vk::MemoryPropertyFlags requirementsMask) {
  uint32_t typeIndex = uint32_t(~0);
  for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
    if ((typeBits & 1) &&
        ((memoryProperties.memoryTypes[i].propertyFlags & requirementsMask) == requirementsMask)) {
      typeIndex = i;
      break;
    }
    typeBits >>= 1;
  }
  assert(typeIndex != uint32_t(~0));
  return typeIndex;
}

}
