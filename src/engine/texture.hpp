#pragma once

#include <memory>

#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_raii.hpp"
#include "utils.hpp"

#include "./buffer.hpp"
#include "./image.hpp"

namespace engine {

class Texture {
public:
    Texture(
        vk::PhysicalDevice const & physicalDevice,
        vk::Device const &         device,
        vk::Extent2D const &       extent            = { 256, 256 },
        vk::ImageUsageFlags        usageFlags         = {},
        vk::FormatFeatureFlags     formatFeatureFlags = {},
        bool                       anisotropyEnable   = false,
        bool                       forceStaging       = false
    );
    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    void clear( vk::Device const & device )
    {
        if ( _stagingBuffer )
        {
            _stagingBuffer->clear( device );
        }
        _image->clear( device );
        device.destroySampler( _sampler );
    }

    template <typename ImageGenerator>
    void setImage( vk::Device const & device, vk::CommandBuffer const & commandBuffer, ImageGenerator const & imageGenerator )
    {
        void * data = _needsStaging
                      ? device.mapMemory( _stagingBuffer->getDeviceMemory(), 0, device.getBufferMemoryRequirements( _stagingBuffer->getBuffer() ).size )
                      : device.mapMemory( _image->getDeviceMemory(), 0, device.getImageMemoryRequirements( _image->getImage() ).size );
        imageGenerator( data, _extent );
        device.unmapMemory( _needsStaging ? _stagingBuffer->getDeviceMemory() : _image->getDeviceMemory() );

        if ( _needsStaging )
        {
          // Since we're going to blit to the texture image, set its layout to eTransferDstOptimal
          engine::setImageLayout( commandBuffer, _image->getImage(), _image->getFormat(), vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal );
          vk::BufferImageCopy copyRegion( 0,
                                          _extent.width,
                                          _extent.height,
                                          vk::ImageSubresourceLayers( vk::ImageAspectFlagBits::eColor, 0, 0, 1 ),
                                          vk::Offset3D( 0, 0, 0 ),
                                          vk::Extent3D( _extent, 1 ) );
          commandBuffer.copyBufferToImage(_stagingBuffer->getBuffer(), _image->getImage(), vk::ImageLayout::eTransferDstOptimal, copyRegion );
          // Set the layout for the texture image from eTransferDstOptimal to SHADER_READ_ONLY
          engine::setImageLayout(
            commandBuffer, _image->getImage(), _image->getFormat(), vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal );
        }
        else
        {
          // If we can use the linear tiled image as a texture, just do it
          engine::setImageLayout(
            commandBuffer, _image->getImage(), _image->getFormat(), vk::ImageLayout::ePreinitialized, vk::ImageLayout::eShaderReadOnlyOptimal );
        }
    }
    
        void setImageLayout(
      vk::CommandBuffer const & commandBuffer, vk::Image image, vk::Format format, vk::ImageLayout oldImageLayout, vk::ImageLayout newImageLayout );

    const std::unique_ptr<Image>& getImage() const { return _image; }
    const vk::Sampler& getSampler() const { return _sampler; }

private:
      vk::Format                  _format;
      vk::Extent2D                _extent;
      bool                        _needsStaging;
      std::unique_ptr<Buffer> _stagingBuffer;
      std::unique_ptr<Image>  _image;
      vk::Sampler                 _sampler;
};

}
