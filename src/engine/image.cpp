#include "image.hpp"

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
      uint32_t memoryTypeIndex = VulkanUtils::findMemoryType( memoryProperties, memoryRequirements.memoryTypeBits, propertyFlags );
      _deviceMemory = device.allocateMemory( vk::MemoryAllocateInfo( memoryRequirements.size, memoryTypeIndex ) );
      

      device.bindImageMemory( _image, _deviceMemory, 0 );

      vk::ImageViewCreateInfo imageViewCreateInfo( {}, _image, vk::ImageViewType::e2D, _format, {}, { aspectMask, 0, 1, 0, 1 } );
      _imageView = device.createImageView( imageViewCreateInfo );
}

Image::~Image() {

}


const vk::Format& Image::getFormat() const {
    return _format;
}

const vk::ImageView& Image::getImageView() const {
    return _imageView;
}

void Image::clear( vk::Device const & device )
{
    device.destroyImageView( _imageView );
    device.destroyImage( _image );  // the Image should to be destroyed before the bound DeviceMemory is freed
    device.freeMemory( _deviceMemory );
}

}
