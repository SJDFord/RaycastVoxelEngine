#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_raii.hpp"

#include "glslang/SPIRV/GlslangToSpv.h"
#include "glslang/Public/ResourceLimits.h"
#include "glslang/Public/ShaderLang.h"

namespace engine {

class RenderPassBuilder {
 public:
  RenderPassBuilder(
    vk::Device const & device
  );
  ~RenderPassBuilder();

  RenderPassBuilder(const RenderPassBuilder&) = delete;
  RenderPassBuilder& operator=(const RenderPassBuilder&) = delete;


  RenderPassBuilder& setColorFormat(vk::Format colorFormat);
  RenderPassBuilder& setDepthFormat(vk::Format depthFormat);

  vk::RenderPass build();

 private:
  vk::Format _colorFormat;
  vk::Format _depthFormat;


  vk::Device _device;
};

}  // namespace engine