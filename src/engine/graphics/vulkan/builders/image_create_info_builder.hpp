#pragma once

#include "vulkan/vulkan.hpp"

#include <iostream>
#include <string>
#include <vector>

namespace engine {

class ImageCreateInfoBuilder {
 public:
  ImageCreateInfoBuilder();
  ~ImageCreateInfoBuilder();

  ImageCreateInfoBuilder(const ImageCreateInfoBuilder&) = delete;
  ImageCreateInfoBuilder& operator=(const ImageCreateInfoBuilder&) = delete;

  ImageCreateInfoBuilder& setExtent(vk::Extent2D extent);
  ImageCreateInfoBuilder& setFormat(vk::Format format);
  ImageCreateInfoBuilder& setUsageFlags(vk::ImageUsageFlags usageFlags);
  ImageCreateInfoBuilder& setSharingMode(vk::SharingMode sharingMode);
  
  vk::ImageCreateInfo build();

 private:
    vk::Extent2D _extent;
    vk::Format _format;
    vk::ImageUsageFlags _usageFlags;
    vk::SharingMode _sharingMode;
};
}  // namespace engine