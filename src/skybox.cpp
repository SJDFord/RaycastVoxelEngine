#include "skybox.hpp"

Skybox::Skybox(lve::LveDevice &device, const std::string &filepath) : lveDevice{device} {
  /*
  int texWidth, texHeight, texChannels;
  std::string pathPrefix = "textures/skybox/water_scene/";
  std::string pathSuffix = "front.jpg";
  std::string path = pathPrefix + pathSuffix;
  stbi_uc *pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
  VkDeviceSize imageSize = texWidth * texHeight * 4;
  if (!pixels) {
    throw std::runtime_error("failed to load texture image!");
  }

  std::unique_ptr<lve::LveBuffer> buffer = std::make_unique<lve::LveBuffer>(
      device,
      imageSize,
      1,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  buffer->map(imageSize, 0);
  buffer->writeToBuffer(pixels, static_cast<size_t>(imageSize));
  buffer->unmap();
  stbi_image_free(pixels);

  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = static_cast<uint32_t>(texWidth);
  imageInfo.extent.height = static_cast<uint32_t>(texHeight);
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;

  imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
  imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.flags = 0;  // Optional

  device.createImageWithInfo(
      imageInfo,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      textureImage,
      textureImageMemory);

  device.transitionImageLayout(
      textureImage,
      VK_FORMAT_R8G8B8A8_SRGB,
      VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  device.copyBufferToImage(
      buffer->getBuffer(),
      textureImage,
      static_cast<uint32_t>(texWidth),
      static_cast<uint32_t>(texHeight),
      1);
  device.transitionImageLayout(
      textureImage,
      VK_FORMAT_R8G8B8A8_SRGB,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  */
}


Skybox::~Skybox() {
  lveDevice.destroyImage(textureImage);
  lveDevice.freeMemory(textureImageMemory);
}


static void loadCubemap(lve::LveDevice &device, std::string filename, VkFormat format) {

}
