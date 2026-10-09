#pragma once

#include "./engine/instance_builder.hpp"
#include "./engine/ranked_physical_device_strategy.hpp"
#include "./engine/device_builder.hpp"
#include "./engine/game_object.hpp"
#include "./engine/renderer.hpp"
#include "./engine/window.hpp"
#include "./engine/renderer.hpp"
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_vulkan.h>
#include "./engine/one_time_command_submitter.hpp"

// std
#include <memory>
#include <vector>

namespace lve {
class VoxelApp {
 public:
  static constexpr std::string APP_NAME = "Voxel App";
  static constexpr std::string ENGINE_NAME = "rayvox";
  static constexpr int WIDTH = 1920;
  static constexpr int HEIGHT = 1080;
  static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

  VoxelApp();
  ~VoxelApp();

  VoxelApp(const VoxelApp &) = delete;
  VoxelApp &operator=(const VoxelApp &) = delete;

  void run();

 private:
  void loadGameObjects();


  engine::Window _window{APP_NAME, WIDTH, HEIGHT};
  vk::Instance _instance = engine::InstanceBuilder(APP_NAME, ENGINE_NAME, VK_API_VERSION_1_3)
    .setExtensions(_window.getRequiredExtensions())
    .build();

  vk::SurfaceKHR _surface{_window.createSurface(_instance)};
  vk::PhysicalDevice _physicalDevice{
    engine::RankedPhysicalDeviceStrategy()
        .pickPhysicalDevice(_instance.enumeratePhysicalDevices())
  };

  std::pair<uint32_t, uint32_t> _queueFamilyIndices{engine::findGraphicsAndPresentQueueFamilyIndex(_physicalDevice, _surface)};
  uint32_t _graphicsQueueIndex{_queueFamilyIndices.first};
  uint32_t _presentQueueIndex{_queueFamilyIndices.second};

  vk::Device _device{engine::DeviceBuilder(_physicalDevice, _graphicsQueueIndex)
    .setExtensions({
        VK_KHR_SWAPCHAIN_EXTENSION_NAME, 
        //VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME
    })
    //.setPNext(new vk::PhysicalDeviceDynamicRenderingFeatures(VK_TRUE))
    .build()};
  vk::CommandPool _commandPool{_device.createCommandPool({vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _graphicsQueueIndex})};

  //LveDevice lveDevice{_window};
  engine::OneTimeCommandSubmitter _oneTimeCommandSubmitter{_device, _commandPool, _graphicsQueueIndex};
  engine::Renderer _renderer{_window, _surface,  _device, _physicalDevice, _commandPool, _graphicsQueueIndex, _presentQueueIndex, MAX_FRAMES_IN_FLIGHT};

  // note: order of declarations matters
  vk::DescriptorPool _globalPool;
  engine::GameObject::Map gameObjects;
};
}  // namespace lve
