#include "image.hpp"

namespace engine {
Image::Image(
    vk::Device const &         device,
    vk::PhysicalDevice const & physicalDevice,
    engine::OneTimeCommandSubmitter& oneTimeCommandSubmitter,
    const std::string &filepath) : _device{device}, _physicalDevice{physicalDevice} {
    createImage(oneTimeCommandSubmitter, filepath);
    createImageView();

    vk::SamplerCreateInfo samplerInfo = vk::SamplerCreateInfo(vk::SamplerCreateFlags())
      .setMagFilter(vk::Filter::eLinear)
      .setMinFilter(vk::Filter::eLinear)
      .setAddressModeU(vk::SamplerAddressMode::eRepeat)
      .setAddressModeV(vk::SamplerAddressMode::eRepeat)
      .setAddressModeW(vk::SamplerAddressMode::eRepeat)
      .setAnisotropyEnable(false)
      .setMaxAnisotropy(1.0f)
      .setBorderColor(vk::BorderColor::eIntOpaqueBlack)
      .setUnnormalizedCoordinates(false)
      .setCompareEnable(false)
      .setCompareOp(vk::CompareOp::eAlways)
      .setMipmapMode(vk::SamplerMipmapMode::eLinear)
      .setMipLodBias(0.0f)
      .setMinLod(0.0f)
      .setMaxLod(0.0f);

    _textureSampler = _device.createSampler(samplerInfo);
}

Image::~Image() {
    _device.destroySampler(_textureSampler);
    _device.destroyImageView(_textureImageView);
    _device.destroyImage(_textureImage);
    _device.freeMemory(_textureImageMemory);
}


void Image::createImage(engine::OneTimeCommandSubmitter& oneTimeCommandSubmitter, const std::string &filepath) {
  int texWidth, texHeight, texChannels;
  stbi_uc *pixels = stbi_load(filepath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
  vk::DeviceSize imageSize = texWidth * texHeight * 4;
  if (!pixels) {
    throw std::runtime_error("failed to load texture image!");
  }

  std::unique_ptr<engine::Buffer> buffer = std::make_unique<engine::Buffer>(
      _device,
      _physicalDevice,
      imageSize,
      1,
      vk::BufferUsageFlagBits::eTransferSrc,
      vk::MemoryPropertyFlagBits::eHostVisible | 
      vk::MemoryPropertyFlagBits::eHostCoherent);

  buffer->map(imageSize, 0);
  buffer->writeToBuffer(pixels, static_cast<size_t>(imageSize));
  buffer->unmap();
  stbi_image_free(pixels);

  vk::Extent2D imageExtent(static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

  vk::ImageCreateInfo imageInfo = vk::ImageCreateInfo()
    .setImageType(vk::ImageType::e2D)
    .setExtent(vk::Extent3D(imageExtent, 1))
    .setMipLevels(1)
    .setArrayLayers(1)
    .setFormat(vk::Format::eR8G8B8A8Srgb)
    .setTiling(vk::ImageTiling::eOptimal)
    .setInitialLayout(vk::ImageLayout::eUndefined)
    .setUsage(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled)
    .setSamples(vk::SampleCountFlagBits::e1)
    .setFlags({});


  _textureImage = _device.createImage(imageInfo);
  vk::MemoryRequirements memRequirements = _device.getImageMemoryRequirements(_textureImage);

  uint32_t memoryTypeIndex = engine::findMemoryType(
    _physicalDevice.getMemoryProperties(), 
    memRequirements.memoryTypeBits, 
    vk::MemoryPropertyFlagBits::eDeviceLocal
  );

  vk::MemoryAllocateInfo allocInfo = vk::MemoryAllocateInfo()
    .setAllocationSize(memRequirements.size)
    .setMemoryTypeIndex(memoryTypeIndex);

  _textureImageMemory = _device.allocateMemory(allocInfo);

  _device.bindImageMemory(_textureImage, _textureImageMemory, 0);

  transitionImageLayout(
      oneTimeCommandSubmitter,
      _textureImage,
      vk::Format::eR8G8B8A8Srgb,
      vk::ImageLayout::eUndefined,
      vk::ImageLayout::eTransferDstOptimal);

  oneTimeCommandSubmitter.submit([&] (const vk::CommandBuffer &commandBuffer) {
    buffer->copyTo(commandBuffer, _textureImage, imageExtent);
  });

  transitionImageLayout(
      oneTimeCommandSubmitter,
      _textureImage,
      vk::Format::eR8G8B8A8Srgb,
      vk::ImageLayout::eTransferDstOptimal,
      vk::ImageLayout::eShaderReadOnlyOptimal);
}

void Image::createImageView() {
    vk::ImageSubresourceRange subresourceRange = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor)
        .setBaseMipLevel(0)
        .setLevelCount(1)
        .setBaseArrayLayer(0)
        .setLayerCount(1);

    vk::ImageViewCreateInfo viewInfo = vk::ImageViewCreateInfo()
        .setImage(_textureImage)
        .setViewType(vk::ImageViewType::e2D)
        .setFormat(vk::Format::eR8G8B8A8Srgb)
        .setSubresourceRange(subresourceRange);

    _textureImageView = _device.createImageView(viewInfo);
}

vk::DescriptorImageInfo Image::descriptorInfo() {
  vk::DescriptorImageInfo imageInfo = vk::DescriptorImageInfo()
      .setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
      .setImageView(_textureImageView)
      .setSampler(_textureSampler);
  return imageInfo;
}

void Image::transitionImageLayout(engine::OneTimeCommandSubmitter& oneTimeCommandSubmitter,
    vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) {
  vk::ImageSubresourceRange subresource = vk::ImageSubresourceRange()
      .setAspectMask(vk::ImageAspectFlagBits::eColor)
      .setBaseMipLevel(0)
      .setLevelCount(1)
      .setBaseArrayLayer(0)
      .setLayerCount(1);
  
  vk::ImageMemoryBarrier barrier = vk::ImageMemoryBarrier()
    .setOldLayout(oldLayout)
    .setNewLayout(newLayout)
    .setSrcQueueFamilyIndex(vk::QueueFamilyIgnored)
    .setDstQueueFamilyIndex(vk::QueueFamilyIgnored)
    .setImage(image)
    .setSubresourceRange(subresource);

  vk::PipelineStageFlags sourceStage;
  vk::PipelineStageFlags destinationStage;

  if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
    barrier.srcAccessMask = vk::AccessFlagBits::eNone;
    barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

    sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
    destinationStage = vk::PipelineStageFlagBits::eTransfer;
  } else if (
      oldLayout == vk::ImageLayout::eTransferDstOptimal &&
      newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
    barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;;

    sourceStage = vk::PipelineStageFlagBits::eTransfer;
    destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
  } else {
    throw std::invalid_argument("unsupported layout transition!");
  }

  oneTimeCommandSubmitter.submit([sourceStage, destinationStage, barrier](const vk::CommandBuffer& commandBuffer) {
    commandBuffer.pipelineBarrier(sourceStage, destinationStage, vk::DependencyFlags(), {}, {}, barrier);
  });
}

}