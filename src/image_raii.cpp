#include "image_raii.hpp"

ImageRaii::ImageRaii(std::shared_ptr<Device> device, const std::string &filepath) : device{device} {
  createImage();
  createImageView();
  device->createSampler(textureSampler);
}

ImageRaii::~ImageRaii() {
  /*
  lveDevice.destroySampler(textureSampler);
  lveDevice.destroyImageView(textureImageView);
  lveDevice.destroyImage(textureImage);
  lveDevice.freeMemory(textureImageMemory);
  */
}

void ImageRaii::createImage() {
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

  Buffer buffer(
      device,
      imageSize,
      1,
      vk::BufferUsageFlagBits::eTransferSrc,
      vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

  buffer.map(imageSize, 0);
  buffer.writeToBuffer(pixels, static_cast<size_t>(imageSize));
  buffer.unmap();
  stbi_image_free(pixels);

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

  auto result = device->createImageWithInfo(
      imageInfo,
      vk::MemoryPropertyFlagBits::eDeviceLocal);

  result.image.swap(textureImage);
  result.imageMemory.swap(textureImageMemory);

  device->transitionImageLayout(
      *textureImage,
      vk::Format::eR8G8B8A8Srgb,
      vk::ImageLayout::eUndefined,
      vk::ImageLayout::eTransferDstOptimal);
  device->copyBufferToImage(
      buffer.getBuffer(),
      *textureImage,
      static_cast<uint32_t>(texWidth),
      static_cast<uint32_t>(texHeight),
      1);
  device->transitionImageLayout(
      *textureImage,
      vk::Format::eR8G8B8A8Srgb,
      vk::ImageLayout::eTransferDstOptimal,
      vk::ImageLayout::eShaderReadOnlyOptimal);
}

void ImageRaii::createImageView() {
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

  device->createImageView(*textureImage, vk::Format::eR8G8B8A8Srgb, textureImageView);
}

void ImageRaii::loadCubemap(std::shared_ptr<Device> device, std::string filename, vk::Format format) {
}

vk::raii::ImageView& ImageRaii::getImageView() { return textureImageView; }
vk::raii::Sampler& ImageRaii::getSampler() { return textureSampler; }