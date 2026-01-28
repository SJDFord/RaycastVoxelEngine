#pragma once

#include "stb/stb_image.h"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "buffer_raii.hpp"

// std
#include <iostream>
#include <lve_device.hpp>
#include <memory>
#include <vector>

// Encapsulates ImageRaii and ImageRaii view/sampling
// TODO: Separate ImageRaii creaton and ImageRaii view/sampling
// TODO: Implement cubemap support for skyboxes
class ImageRaii {
 public:
  // TODO: Other ways to create an ImageRaii
  ImageRaii(
        vk::Device const & device
  );
  ~ImageRaii();

  ImageRaii(const ImageRaii &) = delete;
  ImageRaii &operator=(const ImageRaii &) = delete;

  //vk::ImageView& getImageView();
  //vk::Sampler& getSampler();

 private:
  std::shared_ptr<vk::raii::Image> textureImage = VK_NULL_HANDLE;
  std::shared_ptr<vk::raii::DeviceMemory> textureImageMemory = VK_NULL_HANDLE;
  vk::raii::ImageView textureImageView = VK_NULL_HANDLE;
  vk::raii::Sampler textureSampler = VK_NULL_HANDLE;

  void createImage(const vk::Device & device);
  void createImageView();
};
