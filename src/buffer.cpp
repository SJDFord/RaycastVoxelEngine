/*
 * Encapsulates a vulkan buffer
 *
 * Initially based off VulkanBuffer by Sascha Willems -
 * https://github.com/SaschaWillems/Vulkan/blob/master/base/VulkanBuffer.h
 */

#include "buffer.hpp"

// std
#include <cassert>
#include <cstring>
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
    Device& device,
    vk::DeviceSize instanceSize,
    uint32_t instanceCount,
    vk::BufferUsageFlags usageFlags,
    vk::MemoryPropertyFlags memoryPropertyFlags,
    vk::DeviceSize minOffsetAlignment)
    : _device{device},
      instanceSize{instanceSize},
      instanceCount{instanceCount},
      usageFlags{usageFlags},
      memoryPropertyFlags{memoryPropertyFlags} {
  alignmentSize = getAlignment(instanceSize, minOffsetAlignment);
  bufferSize = alignmentSize * instanceCount;
  
  vk::BufferCreateInfo bufferInfo = vk::BufferCreateInfo()
    .setSize(bufferSize)
    .setUsage(usageFlags)
    .setSharingMode(vk::SharingMode::eExclusive);
  _buffer = device.device().createBuffer(bufferInfo);

  vk::DeviceBufferMemoryRequirements memoryRequirementsInfo =
      vk::DeviceBufferMemoryRequirements().setPCreateInfo(&bufferInfo);
  vk::MemoryRequirements memoryRequirements = _buffer.getMemoryRequirements();
  
  vk::MemoryAllocateInfo allocInfo =
      vk::MemoryAllocateInfo()
          .setAllocationSize(memoryRequirements.size)
          .setMemoryTypeIndex(
              device.findMemoryType(memoryRequirements.memoryTypeBits, memoryPropertyFlags));
 
  _memory = vk::raii::DeviceMemory(_device.device(), allocInfo);
  _buffer.bindMemory(_memory, 0);
}

Buffer::~Buffer() {
  unmap();
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
void Buffer::map(vk::DeviceSize size, vk::DeviceSize offset) {
  _mapped = _memory.mapMemory(offset, size, vk::MemoryMapFlags());
}

/**
 * Unmap a mapped memory range
 *
 * @note Does not return a result as vkUnmapMemory can't fail
 */
void Buffer::unmap() {
  if (_mapped) {
    _memory.unmapMemory();
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
    memcpy(_mapped, data, bufferSize);
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
void Buffer::flush(vk::DeviceSize size, vk::DeviceSize offset) {
  vk::MappedMemoryRange mappedRange = vk::MappedMemoryRange()
      .setMemory(*_memory)
      .setOffset(offset)
      .setSize(size);
  _device.device().flushMappedMemoryRanges(mappedRange);
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
void Buffer::invalidate(vk::DeviceSize size, vk::DeviceSize offset) {
  vk::MappedMemoryRange mappedRange = vk::MappedMemoryRange()
      .setMemory(*_memory)
      .setOffset(offset)
      .setSize(size);
  _device.device().invalidateMappedMemoryRanges(mappedRange);
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
  return vk::DescriptorBufferInfo().setBuffer(_buffer).setOffset(offset).setRange(size);
}

/**
 * Copies "instanceSize" bytes of data to the mapped buffer at an offset of index * alignmentSize
 *
 * @param data Pointer to the data to copy
 * @param index Used in offset calculation
 *
 */
void Buffer::writeToIndex(void *data, int index) {
  writeToBuffer(data, instanceSize, index * alignmentSize);
}

/**
 *  Flush the memory range at index * alignmentSize of the buffer to make it visible to the device
 *
 * @param index Used in offset calculation
 *
 */
void Buffer::flushIndex(int index) { return flush(alignmentSize, index * alignmentSize); }

/**
 * Create a buffer info descriptor
 *
 * @param index Specifies the region given by index * alignmentSize
 *
 * @return VkDescriptorBufferInfo for instance at index
 */
vk::DescriptorBufferInfo Buffer::descriptorInfoForIndex(int index) {
  return descriptorInfo(alignmentSize, index * alignmentSize);
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
void Buffer::invalidateIndex(int index) {
  return invalidate(alignmentSize, index * alignmentSize);
}
