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

Buffer::Buffer(
    const vk::PhysicalDevice& physicalDevice,      
    const vk::Device& device,
    vk::DeviceSize size,
    vk::BufferUsageFlags usage,
    vk::MemoryPropertyFlags propertyFlags
    )
    : _device{device},
      _size{size},
      _usage{usage},
      _propertyFlags{propertyFlags} {

    vk::BufferCreateInfo bufferInfo = vk::BufferCreateInfo()
    .setSize(size)
    .setUsage(usage)
    .setSharingMode(vk::SharingMode::eExclusive);
    _buffer = device.createBuffer(bufferInfo);

    vk::DeviceBufferMemoryRequirements memoryRequirementsInfo =
      vk::DeviceBufferMemoryRequirements().setPCreateInfo(&bufferInfo);
    vk::MemoryRequirements memoryRequirements = device.getBufferMemoryRequirements(_buffer);

    vk::PhysicalDeviceMemoryProperties memoryProperties = physicalDevice.getMemoryProperties();
    uint32_t memoryTypeIndex = engine::findMemoryType( memoryProperties, memoryRequirements.memoryTypeBits, propertyFlags );

    vk::MemoryAllocateInfo allocInfo =
      vk::MemoryAllocateInfo()
          .setAllocationSize(memoryRequirements.size)
          .setMemoryTypeIndex(memoryTypeIndex);
    _memory = _device.allocateMemory(allocInfo);
    _device.bindBufferMemory(_buffer, _memory, 0);
}

Buffer::~Buffer() {
    unmap();
    clear(_device);
    //clear();
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
  assert(_buffer && _memory && "Called map on buffer before create");
  _mapped = _device.mapMemory(_memory, offset, size, {});
}


void Buffer::writeToBuffer(void* data, vk::DeviceSize size, vk::DeviceSize offset) {
  assert(_mapped && "Cannot copy to unmapped buffer");

  if (size == VK_WHOLE_SIZE) {
    memcpy(_mapped, data, _size);
  } else {
    char *memOffset = (char *)_mapped;
    memOffset += offset;
    memcpy(memOffset, data, size);
  }
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

void Buffer::flush(vk::DeviceSize size, vk::DeviceSize offset) {
  vk::MappedMemoryRange mappedRange = vk::MappedMemoryRange()
    .setMemory(_memory)
    .setOffset(offset)
    .setSize(size);
  _device.flushMappedMemoryRanges(mappedRange);
}

void Buffer::clear( vk::Device const & device )
{
    device.destroyBuffer( _buffer );  // to prevent some validation layer warning, the Buffer needs to be destroyed before the bound DeviceMemory
    device.freeMemory( _memory );
}

vk::DescriptorBufferInfo Buffer::descriptorInfo(vk::DeviceSize size, vk::DeviceSize offset) {
  vk::DescriptorBufferInfo info = vk::DescriptorBufferInfo()
    .setBuffer(_buffer)
    .setOffset(size)
    .setRange(offset);

  return info;
}

}
