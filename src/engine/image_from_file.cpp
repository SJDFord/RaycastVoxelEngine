#include "image_from_file.hpp"

#include <iostream>
#include <print>


namespace engine {

ImageFromFile::ImageFromFile( 
        vk::PhysicalDevice const & physicalDevice,
        vk::Device const &         device,
        vk::CommandBuffer const & commandBuffer,
        const std::string &filepath
) : _format( vk::Format::eR8G8B8A8Srgb ) {
      
      createImage(device, physicalDevice, commandBuffer, filepath);
      vk::ImageViewCreateInfo imageViewCreateInfo( {}, _image, vk::ImageViewType::e2D, _format, {}, { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 } );
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

ImageFromFile::~ImageFromFile() {

}

void ImageFromFile::createImage(
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

  std::printf("Loaded %s (%ix%i) \n", path.c_str(), texWidth, texHeight);

  std::unique_ptr<engine::Buffer> buffer = std::make_unique<engine::Buffer>(
      physicalDevice,
      device,
      imageSize,
      vk::BufferUsageFlagBits::eTransferSrc,
      vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

  //buffer->map(imageSize, 0);
  //std::println("Mapped image to buffer");
  buffer->map(imageSize, 0);
  buffer->writeToBuffer(pixels, static_cast<size_t>(imageSize));
  buffer->unmap();
  std::println("Pixels written to buffer");
  //buffer->unmap();
  //std::println("Buffer unmapped");
  stbi_image_free(pixels);

  // Image
  vk::ImageCreateInfo imageCreateInfo( vk::ImageCreateFlags(),
                                        vk::ImageType::e2D,
                                        _format,
                                        vk::Extent3D( static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1 ),
                                        1,
                                        1,
                                        vk::SampleCountFlagBits::e1,
                                        vk::ImageTiling::eOptimal,
                                        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
                                        vk::SharingMode::eExclusive, 
                                        {},
                                        vk::ImageLayout::eUndefined
                                     );
   _image = device.createImage( imageCreateInfo );

    vk::MemoryRequirements memoryRequirements = device.getImageMemoryRequirements(_image);
    vk::PhysicalDeviceMemoryProperties memoryProperties = physicalDevice.getMemoryProperties();
    uint32_t memoryTypeIndex = engine::findMemoryType( memoryProperties, memoryRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal );
    _deviceMemory = device.allocateMemory( vk::MemoryAllocateInfo( memoryRequirements.size, memoryTypeIndex ) );

    device.bindImageMemory( _image, _deviceMemory, 0 );

    setImageLayout(
      commandBuffer,
      _image,
      _format,
      vk::ImageLayout::eUndefined,
      vk::ImageLayout::eTransferDstOptimal
    );

    copyBufferToImage(
        commandBuffer,
        buffer->getBuffer(),
        _image,
        static_cast<uint32_t>(texWidth),
        static_cast<uint32_t>(texHeight),
        1
    );

    setImageLayout(
      commandBuffer,
      _image,
      _format,
      vk::ImageLayout::eTransferDstOptimal,
      vk::ImageLayout::eShaderReadOnlyOptimal
    );
}

const vk::Format& ImageFromFile::getFormat() const {
    return _format;
}

const vk::ImageView& ImageFromFile::getImageView() const {
    return _imageView;
}

vk::Image ImageFromFile::getImage() const {
    return _image;
}


const vk::DeviceMemory& ImageFromFile::getDeviceMemory() const {
    return _deviceMemory;
}


const vk::Sampler& ImageFromFile::getSampler() const {
    return _sampler;
}

void ImageFromFile::clear( vk::Device const & device )
{
    device.destroyImageView( _imageView );
    device.destroyImage( _image );  // the Image should to be destroyed before the bound DeviceMemory is freed
    device.freeMemory( _deviceMemory );
}

}
