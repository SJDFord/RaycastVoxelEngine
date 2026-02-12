#pragma once

#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_raii.hpp"

#include "utils.hpp"

namespace engine {

class Image
{
public:
    Image( 
        vk::PhysicalDevice const & physicalDevice,
        vk::Device const &         device,
        vk::Format                 format,
        vk::Extent2D const &       extent,
        vk::ImageTiling            tiling,
        vk::ImageUsageFlags        usage,
        vk::ImageLayout            initialLayout,
        vk::MemoryPropertyFlags    propertyFlags,
        vk::ImageAspectFlags       aspectMask 
    );
    ~Image();

    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;
    
    const vk::Format& getFormat() const;
    const vk::ImageView& getImageView() const;
    const vk::Image& getImage() const;
    const vk::DeviceMemory& getDeviceMemory() const;

    void clear( vk::Device const & device );

private:
    vk::Format       _format;
    vk::Image        _image;
    vk::DeviceMemory _deviceMemory;
    vk::ImageView    _imageView;
};

}
