#pragma once

#include "one_time_command_submitter.hpp"
#include "buffer.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <functional>
#include <iostream>
#include <memory>
#include <vector>
namespace engine {
// Encapsulates image and image view/sampling 
// TODO: Separate image creaton and image view/sampling
class Image {
 public:
  Image(
    vk::Device const &         device,
    vk::PhysicalDevice const & physicalDevice,
    OneTimeCommandSubmitter& oneTimeCommandSubmitter,
    void* data,
    uint32_t dataSize,
    vk::Extent2D extent
  );
 
  // TODO: Other ways to create an image
  Image(
    vk::Device const &         device,
    vk::PhysicalDevice const & physicalDevice,
    OneTimeCommandSubmitter& oneTimeCommandSubmitter,
    void* data,
    uint32_t dataSize,
    vk::Extent2D extent,
    uint32_t mipLevels,
    std::function<uint32_t(uint32_t mipLevel, uint32_t layer, uint32_t faceSlice)> getOffset,
    std::function<uint32_t(uint32_t mipLevel)> getImageSize
    //const std::string &filepath
  );
  ~Image();

  Image(const Image &) = delete;
  Image &operator=(const Image &) = delete;

  vk::DescriptorImageInfo descriptorInfo();

 private:
  vk::Device _device;
  vk::PhysicalDevice _physicalDevice;
  vk::Image _textureImage = VK_NULL_HANDLE;
  vk::DeviceMemory _textureImageMemory = VK_NULL_HANDLE;
  vk::ImageView _textureImageView = VK_NULL_HANDLE;
  vk::Sampler _textureSampler = VK_NULL_HANDLE;

  void transitionImageLayout(
    OneTimeCommandSubmitter& oneTimeCommandSubmitter, vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t levelCount = 1);
};

}
