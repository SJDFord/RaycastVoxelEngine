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

class SwapchainBuilder {
 public:
  SwapchainBuilder(
    vk::Device const & device
  );
  ~SwapchainBuilder();

  SwapchainBuilder(const SwapchainBuilder&) = delete;
  SwapchainBuilder& operator=(const SwapchainBuilder&) = delete;


  SwapchainBuilder& setSurfaceFormat(vk::SurfaceFormatKHR surfaceFormat);
  SwapchainBuilder& setPresentMode(vk::PresentModeKHR presentMode);
  SwapchainBuilder& setExtent(vk::Extent2D extent);
  SwapchainBuilder& setGraphicsQueueFamilyIndex(uint32_t queueFamilyIndex);
  SwapchainBuilder& setPresentQueueFamilyIndex(uint32_t queueFamilyIndex);
  SwapchainBuilder& setOldSwapchain(vk::SwapchainKHR oldSwapchain);
  SwapchainBuilder& setSurfaceCapabilities(vk::SurfaceCapabilitiesKHR capabilities);
  SwapchainBuilder& setSurface(vk::SurfaceKHR surface);

  vk::SwapchainKHR build();

 private:
  vk::Device _device;
  vk::SurfaceFormatKHR _surfaceFormat;
  vk::PresentModeKHR _presentMode;
  vk::Extent2D _extent;
  uint32_t _graphicsQueueFamilyIndex;
  uint32_t _presentQueueFamilyIndex;
  vk::SwapchainKHR _oldSwapchain;
  vk::SurfaceCapabilitiesKHR _capabilities;
  vk::SurfaceKHR _surface;
};

}  // namespace engine