#pragma once

#include "vulkan/vulkan.hpp"

#include <iostream>
#include <string>
#include <vector>

namespace engine {

class ImageViewCreateInfoBuilder {
 public:
  ImageViewCreateInfoBuilder();
  ~ImageViewCreateInfoBuilder();

  ImageViewCreateInfoBuilder(const ImageViewCreateInfoBuilder&) = delete;
  ImageViewCreateInfoBuilder& operator=(const ImageViewCreateInfoBuilder&) = delete;

  ImageViewCreateInfoBuilder& setFlags(vk::ImageAspectFlags flags);
  ImageViewCreateInfoBuilder& setImage(vk::Image image);
  ImageViewCreateInfoBuilder& setFormat(vk::Format format);
  vk::ImageViewCreateInfo build();

 private:
    vk::ImageAspectFlags _flags;
    vk::Image _image;
    vk::Format _format;

};
}  // namespace engine
