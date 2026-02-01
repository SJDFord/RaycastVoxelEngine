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
      size{size},
      usage{usage},
      propertyFlags{propertyFlags} {

    vk::BufferCreateInfo bufferInfo = vk::BufferCreateInfo()
    .setSize(size)
    .setUsage(usage)
    .setSharingMode(vk::SharingMode::eExclusive);
    _buffer = device.createBuffer(bufferInfo);

    vk::DeviceBufferMemoryRequirements memoryRequirementsInfo =
      vk::DeviceBufferMemoryRequirements().setPCreateInfo(&bufferInfo);
    vk::MemoryRequirements memoryRequirements = device.getBufferMemoryRequirements(_buffer);

    vk::PhysicalDeviceMemoryProperties memoryProperties = physicalDevice.getMemoryProperties();
    uint32_t memoryTypeIndex = findMemoryType( memoryProperties, memoryRequirements.memoryTypeBits, propertyFlags );

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

uint32_t BufferRaii::findMemoryType( 
    vk::PhysicalDeviceMemoryProperties const & memoryProperties, 
    uint32_t typeBits, 
    vk::MemoryPropertyFlags requirementsMask 
    ) {
    uint32_t typeIndex = uint32_t( ~0 );
    for ( uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++ )
    {
    if ( ( typeBits & 1 ) && ( ( memoryProperties.memoryTypes[i].propertyFlags & requirementsMask ) == requirementsMask ) )
    {
      typeIndex = i;
      break;
    }
    typeBits >>= 1;
    }
    assert( typeIndex != uint32_t( ~0 ) );
    return typeIndex;
}

