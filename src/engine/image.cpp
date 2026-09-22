#include "image.hpp"

#include <iostream>

namespace engine {

Image::Image( 
    vk::PhysicalDevice const & physicalDevice,
    vk::Device const &         device,
    vk::Format                 format,
    vk::Extent2D const &       extent,
    vk::ImageTiling            tiling,
    vk::ImageUsageFlags        usage,
    vk::ImageLayout            initialLayout,
    vk::MemoryPropertyFlags    propertyFlags,
    vk::ImageAspectFlags       aspectMask 
) : _format( format ) {

    vk::ImageCreateInfo imageCreateInfo( vk::ImageCreateFlags(),
                                           vk::ImageType::e2D,
                                           format,
                                           vk::Extent3D( extent, 1 ),
                                           1,
                                           1,
                                           vk::SampleCountFlagBits::e1,
                                           tiling,
                                           usage | vk::ImageUsageFlagBits::eSampled,
                                           vk::SharingMode::eExclusive,
                                           {},
                                           initialLayout );
      _image = device.createImage( imageCreateInfo );
      
      vk::MemoryRequirements memoryRequirements = device.getImageMemoryRequirements(_image);
      vk::PhysicalDeviceMemoryProperties memoryProperties = physicalDevice.getMemoryProperties();
      uint32_t memoryTypeIndex = engine::findMemoryType( memoryProperties, memoryRequirements.memoryTypeBits, propertyFlags );
      _deviceMemory = device.allocateMemory( vk::MemoryAllocateInfo( memoryRequirements.size, memoryTypeIndex ) );
      

      device.bindImageMemory( _image, _deviceMemory, 0 );

      vk::ImageViewCreateInfo imageViewCreateInfo( {}, _image, vk::ImageViewType::e2D, _format, {}, { aspectMask, 0, 1, 0, 1 } );
      _imageView = device.createImageView( imageViewCreateInfo );
      _sampler = device.createSampler( vk::SamplerCreateInfo( vk::SamplerCreateFlags(),
                                                             vk::Filter::eLinear,
                                                             vk::Filter::eLinear,
                                                             vk::SamplerMipmapMode::eLinear,
                                                             vk::SamplerAddressMode::eRepeat,
                                                             vk::SamplerAddressMode::eRepeat,
                                                             vk::SamplerAddressMode::eRepeat,
                                                             0.0f,
                                                             true,
                                                             16.0f,
                                                             false,
                                                             vk::CompareOp::eNever,
                                                             0.0f,
                                                             0.0f,
                                                             vk::BorderColor::eFloatOpaqueBlack ) );
}

Image::~Image() {

}

void Image::setTexture(
        vk::Device const &         device,
        vk::PhysicalDevice const & physicalDevice,
        vk::CommandBuffer const & commandBuffer,
        const std::string& path) {
int texWidth, texHeight, texChannels;
  stbi_uc *pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
  vk::DeviceSize imageSize = texWidth * texHeight * 4;
  if (!pixels) {
    throw std::runtime_error("failed to load texture image!");
  }

  std::unique_ptr<engine::Buffer> buffer = std::make_unique<engine::Buffer>(
      physicalDevice,
      device,
      imageSize,
      vk::BufferUsageFlagBits::eTransferSrc,
      vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

  buffer->map(imageSize, 0);
  buffer->write(pixels, static_cast<size_t>(imageSize));
  buffer->unmap();
  stbi_image_free(pixels);

  /*
  lveDevice.createImageWithInfo(
      imageInfo,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      textureImage,
      textureImageMemory);
  */

  /*
  lveDevice.transitionImageLayout(
      textureImage,
      VK_FORMAT_R8G8B8A8_SRGB,
      VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
   */
   copyBufferToImage(
      commandBuffer,
      buffer->getBuffer(),
      _image,
      static_cast<uint32_t>(texWidth),
      static_cast<uint32_t>(texHeight),
      1);
  /*
  lveDevice.transitionImageLayout(
      textureImage,
      VK_FORMAT_R8G8B8A8_SRGB,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  */
}

const vk::Format& Image::getFormat() const {
    return _format;
}

const vk::ImageView& Image::getImageView() const {
    return _imageView;
}

vk::Image Image::getImage() const {
    return _image;
}


const vk::DeviceMemory& Image::getDeviceMemory() const {
    return _deviceMemory;
}

void Image::clear( vk::Device const & device )
{
    device.destroyImageView( _imageView );
    device.destroyImage( _image );  // the Image should to be destroyed before the bound DeviceMemory is freed
    device.freeMemory( _deviceMemory );
}

}
