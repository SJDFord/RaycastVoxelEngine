#pragma once

#include "../one_time_command_submitter.hpp"
#include "../image.hpp"
#include "vulkan/vulkan.hpp"


#include <ktx.h>
#include <ktxvulkan.h>

//#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include <iostream>
#include <string>
#include <vector>

namespace engine {

class FileImageBuilder {
 public:
  FileImageBuilder(
    vk::Device const &         device,
    vk::PhysicalDevice const & physicalDevice
  );
  ~FileImageBuilder();

  FileImageBuilder(const FileImageBuilder&) = delete;
  FileImageBuilder& operator=(const FileImageBuilder&) = delete;

  std::unique_ptr<engine::Image> build(OneTimeCommandSubmitter& oneTimeCommandSubmitter, std::string filePath);

 private:
  const vk::Device& _device;
  const vk::PhysicalDevice& _physicalDevice;
};
}  // namespace engine