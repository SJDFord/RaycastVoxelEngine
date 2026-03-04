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

class ShaderModuleBuilder {
 public:
  ShaderModuleBuilder(vk::Device const& device, vk::ShaderStageFlagBits shaderStage, std::string glslCode);
  ~ShaderModuleBuilder();

  ShaderModuleBuilder(const ShaderModuleBuilder&) = delete;
  ShaderModuleBuilder& operator=(const ShaderModuleBuilder&) = delete;

  vk::ShaderModule build();

 private:
  vk::Device _device;
  vk::ShaderStageFlagBits _shaderStage;
  std::string _glslCode;

  EShLanguage translateShaderStage(vk::ShaderStageFlagBits stage);
  TBuiltInResource InitResources();
  bool GLSLtoSPV(
    const vk::ShaderStageFlagBits shaderType,
    std::string const& glslShader,
    std::vector<unsigned int>& spvShader);
};

}  // namespace engine