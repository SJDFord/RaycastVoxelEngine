#pragma once

#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_raii.hpp"
#include "../util/vulkan_utils.hpp"

namespace engine {

class Buffer {
public:
    Buffer(
        const vk::PhysicalDevice& physicalDevice,      
        const vk::Device& device,
        vk::DeviceSize size,
        vk::BufferUsageFlags usage,
        vk::MemoryPropertyFlags propertyFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
    );
    ~Buffer();

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    template <class T>
    void write( 
        T const * pData, 
        size_t count, 
        vk::DeviceSize stride = sizeof( T ) ) {
      assert( sizeof( T ) <= stride );
      uint8_t * deviceData = static_cast<uint8_t *>( _device.mapMemory( _memory, 0, count * stride ) );
      if ( stride == sizeof( T ) )
      {
        memcpy( deviceData, pData, count * sizeof( T ) );
      }
      else
      {
        for ( size_t i = 0; i < count; i++ )
        {
          memcpy( deviceData, &pData[i], sizeof( T ) );
          deviceData += stride;
        }
      }
      _device.unmapMemory( _memory );
    }

    template <class T>
    void write( 
        T const & data 
    )
    {
  write<T>( &data, 1 );
}

    void clear();
  
    const vk::Buffer& getBuffer() const { return _buffer; }
    const vk::DeviceMemory& getDeviceMemory() const { return _memory; }

private:
    const vk::Device& _device;
    vk::DeviceSize _size;
    vk::BufferUsageFlags _usage;
    vk::MemoryPropertyFlags _propertyFlags;

    vk::Buffer _buffer;
    vk::DeviceMemory _memory;
};

}
