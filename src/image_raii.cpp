#include "image_raii.hpp"

ImageRaii::ImageRaii(
        vk::Device const & device 
) {
  createImage(device);
  createImageView();
  
  /* Old settings
  VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_FALSE;  // VK_TRUE;
    samplerInfo.maxAnisotropy = 1.0f;
    //properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;
  */
  
  /*
  textureSampler = device.createSampler( vk::SamplerCreateInfo( vk::SamplerCreateFlags(),
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
  */
}

ImageRaii::~ImageRaii() {
  /*
  lveDevice.destroySampler(textureSampler);
  lveDevice.destroyImageView(textureImageView);
  lveDevice.destroyImage(textureImage);
  lveDevice.freeMemory(textureImageMemory);
  */
}

void ImageRaii::createImage(const vk::Device& device) {
  int texWidth, texHeight, texChannels;
  std::string pathPrefix = "textures/skybox/water_scene/";
  std::string pathSuffix = "front.jpg";
  // std::string path = pathPrefix + pathSuffix;
  std::string path = "C:/Users/sjdf/Code/RaycastVoxelEngine/textures/jungle-brick-with-moss.png";
  stbi_uc *pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
  vk::DeviceSize imageSize = texWidth * texHeight * 4;
  if (!pixels) {
    throw std::runtime_error("failed to load texture image!");
  }
  /*
  BufferRaii buffer(
      device,
      imageSize,
      1,
      vk::BufferUsageFlagBits::eTransferSrc,
      vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
     

  buffer.map(imageSize, 0);
  buffer.writeToBuffer(pixels, static_cast<size_t>(imageSize));
  buffer.unmap();
  stbi_image_free(pixels);
  */
  /*
  vk::ImageCreateInfo imageInfo{};
  imageInfo.sType = vk::StructureType::eImageCreateInfo;
  imageInfo.imageType = vk::ImageType::e2D;
  imageInfo.extent.width = static_cast<uint32_t>(texWidth);
  imageInfo.extent.height = static_cast<uint32_t>(texHeight);
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;

  imageInfo.format = vk::Format::eR8G8B8A8Srgb;
  imageInfo.tiling = vk::ImageTiling::eOptimal;
  imageInfo.initialLayout = vk::ImageLayout::eUndefined;
  imageInfo.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
  imageInfo.sharingMode = vk::SharingMode::eExclusive;
  imageInfo.samples = vk::SampleCountFlagBits::e1;
  imageInfo.flags = vk::ImageCreateFlags();  // Optional
  */
  /*
  auto result = device.createImageWithInfo(
      imageInfo,
      vk::MemoryPropertyFlagBits::eDeviceLocal);

  result.image.swap(textureImage);
  result.imageMemory.swap(textureImageMemory);
  */
  
  /*
  TODO: Implement
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
   textureImage = std::make_shared<<vk::raii::Image>(device.createImage( imageCreateInfo ));
      std::cout << "Image created!" << std::endl;
   deviceMemory = vk::su::allocateDeviceMemory( device, physicalDevice.getMemoryProperties(), device.getImageMemoryRequirements( image ), memoryProperties );
      std::cout << "Memory allocated!" << std::endl;

      device.bindImageMemory( image, deviceMemory, 0 );
      std::cout << "Image bound!" << std::endl;

      vk::ImageViewCreateInfo imageViewCreateInfo( {}, image, vk::ImageViewType::e2D, format, {}, { aspectMask, 0, 1, 0, 1 } );
      imageView = device.createImageView( imageViewCreateInfo );
      std::cout << "Image view created!" << std::endl;
  */

  // TODO: Implement
  /*
  device.transitionImageLayout(
      *textureImage,
      vk::Format::eR8G8B8A8Srgb,
      vk::ImageLayout::eUndefined,
      vk::ImageLayout::eTransferDstOptimal);
  device.copyBufferToImage(
      buffer.getBuffer(),
      *textureImage,
      static_cast<uint32_t>(texWidth),
      static_cast<uint32_t>(texHeight),
      1);
  device.transitionImageLayout(
      *textureImage,
      vk::Format::eR8G8B8A8Srgb,
      vk::ImageLayout::eTransferDstOptimal,
      vk::ImageLayout::eShaderReadOnlyOptimal);
  */
}

void ImageRaii::createImageView() {
  /*
  vk::ImageViewCreateInfo viewInfo{};
  viewInfo.sType = vk::StructureType::eImageViewCreateInfo;
  viewInfo.image = *textureImage;
  viewInfo.viewType = vk::ImageViewType::e2D;
  viewInfo.format = vk::Format::eR8G8B8A8Srgb;
  viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = 1;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = 1;

  device.createImageView(*textureImage, vk::Format::eR8G8B8A8Srgb, textureImageView);
  */
}

//vk::ImageView& ImageRaii::getImageView() { return textureImageView; }
//vk::Sampler& ImageRaii::getSampler() { return textureSampler; }
