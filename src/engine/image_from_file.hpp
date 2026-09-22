#pragma once

#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_raii.hpp"

#include "utils.hpp"
#include "buffer.hpp"

#include "stb/stb_image.h"

namespace engine {

class ImageFromFile
{
public:
    ImageFromFile( 
        vk::PhysicalDevice const & physicalDevice,
        vk::Device const &         device,
        vk::CommandBuffer const & commandBuffer,
        const std::string &filepath
    );
    ~ImageFromFile();

    ImageFromFile(const ImageFromFile&) = delete;
    ImageFromFile& operator=(const ImageFromFile&) = delete;
    
    const vk::Format& getFormat() const;
    const vk::ImageView& getImageView() const;
    vk::Image getImage() const;
    const vk::DeviceMemory& getDeviceMemory() const;

    void clear( vk::Device const & device );

private:
    vk::Format       _format;
    vk::Image        _image;
    vk::DeviceMemory _deviceMemory;
    vk::ImageView    _imageView;
    vk::Sampler      _sampler;


    void createImage(        
        vk::Device const &         device,
        vk::PhysicalDevice const & physicalDevice,
        vk::CommandBuffer const & commandBuffer,
        const std::string &filepath
    );
};

}
