#pragma once

#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_raii.hpp"

#include "util/vulkan_utils.hpp"

class ImageRaii
{
public:
    ImageRaii( 
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
    ~ImageRaii();

    ImageRaii(const ImageRaii&) = delete;
    ImageRaii& operator=(const ImageRaii&) = delete;
    
    const vk::Format& getFormat() const;
    const vk::ImageView& getImageView() const;

    void clear( vk::Device const & device );

private:
    vk::Format       _format;
    vk::Image        _image;
    vk::DeviceMemory _deviceMemory;
    vk::ImageView    _imageView;
};
