#include "buffer_raii.hpp"

/*
 * Encapsulates a vulkan buffer
 *
 * Initially based off VulkanBuffer by Sascha Willems -
 * https://github.com/SaschaWillems/Vulkan/blob/master/base/VulkanBuffer.h
 */

// std
#include <cassert>
#include <cstring>

BufferRaii::BufferRaii(
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
    uint32_t memoryTypeIndex = VulkanUtils::findMemoryType( memoryProperties, memoryRequirements.memoryTypeBits, propertyFlags );

    vk::MemoryAllocateInfo allocInfo =
      vk::MemoryAllocateInfo()
          .setAllocationSize(memoryRequirements.size)
          .setMemoryTypeIndex(memoryTypeIndex);
    _memory = _device.allocateMemory(allocInfo);
    _device.bindBufferMemory(_buffer, _memory, 0);
}

BufferRaii::~BufferRaii() {
    //clear();
}

void BufferRaii::clear()
{
    _device.destroyBuffer( _buffer );  // to prevent some validation layer warning, the Buffer needs to be destroyed before the bound DeviceMemory
    _device.freeMemory( _memory );
}

