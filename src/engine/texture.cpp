#include "texture.hpp"

namespace engine {

Texture::Texture(
    vk::PhysicalDevice const & physicalDevice,
    vk::Device const &         device,
    vk::Extent2D const &       extent,
    vk::ImageUsageFlags        usageFlags,
    vk::FormatFeatureFlags     formatFeatureFlags,
    bool                       anisotropyEnable,
    bool                       forceStaging
) : _format( vk::Format::eR8G8B8A8Unorm ), _extent( extent )
 {
      vk::FormatProperties formatProperties = physicalDevice.getFormatProperties( _format );

      formatFeatureFlags |= vk::FormatFeatureFlagBits::eSampledImage;
      _needsStaging = forceStaging || ( ( formatProperties.linearTilingFeatures & formatFeatureFlags ) != formatFeatureFlags );
      vk::ImageTiling         imageTiling;
      vk::ImageLayout         initialLayout;
      vk::MemoryPropertyFlags requirements;
      if ( _needsStaging )
      {
        assert( ( formatProperties.optimalTilingFeatures & formatFeatureFlags ) == formatFeatureFlags );
        _stagingBuffer =
          std::unique_ptr<Buffer>( new Buffer( physicalDevice, device, _extent.width * _extent.height * 4, vk::BufferUsageFlagBits::eTransferSrc ) );
        imageTiling = vk::ImageTiling::eOptimal;
        usageFlags |= vk::ImageUsageFlagBits::eTransferDst;
        initialLayout = vk::ImageLayout::eUndefined;
      }
      else
      {
        imageTiling   = vk::ImageTiling::eLinear;
        initialLayout = vk::ImageLayout::ePreinitialized;
        requirements  = vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible;
      }
      _image = std::unique_ptr<Image>( new Image( physicalDevice,
                                                             device,
                                                             _format,
                                                             _extent,
                                                             imageTiling,
                                                             usageFlags | vk::ImageUsageFlagBits::eSampled,
                                                             initialLayout,
                                                             requirements,
                                                             vk::ImageAspectFlagBits::eColor ) );

      _sampler = device.createSampler( vk::SamplerCreateInfo( vk::SamplerCreateFlags(),
                                                             vk::Filter::eLinear,
                                                             vk::Filter::eLinear,
                                                             vk::SamplerMipmapMode::eLinear,
                                                             vk::SamplerAddressMode::eRepeat,
                                                             vk::SamplerAddressMode::eRepeat,
                                                             vk::SamplerAddressMode::eRepeat,
                                                             0.0f,
                                                             anisotropyEnable,
                                                             16.0f,
                                                             false,
                                                             vk::CompareOp::eNever,
                                                             0.0f,
                                                             0.0f,
                                                             vk::BorderColor::eFloatOpaqueBlack ) );
}

Texture::~Texture() {
}

}
