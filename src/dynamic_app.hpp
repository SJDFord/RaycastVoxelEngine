#pragma once

#include "./engine/window.hpp"
#include "./engine/renderer.hpp"
#include "./engine/descriptor_pool_builder.hpp"
#include "./engine/device_builder.hpp"
#include "./engine/instance_builder.hpp"
#include "./engine/physical_device_strategy.hpp"
#include "./engine/ranked_physical_device_strategy.hpp"

#include "./engine/game_object.hpp"

// std
#include <memory>
#include <vector>
#include <optional>
#include <iostream>

class DynamicApp {
 public:

  static constexpr std::string APP_NAME = "Dynamic App";
  static constexpr std::string ENGINE_NAME = "rayvox";
  static constexpr int WIDTH = 1920;
  static constexpr int HEIGHT = 1080;
  static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

  DynamicApp();
  ~DynamicApp();

  DynamicApp(const DynamicApp &) = delete;
  DynamicApp &operator=(const DynamicApp &) = delete;

  void run();
 private:
  engine::Window _window{APP_NAME, WIDTH, HEIGHT};
  vk::Instance _instance{engine::InstanceBuilder(APP_NAME, ENGINE_NAME, VK_API_VERSION_1_3)
    .setExtensions(_window.getRequiredExtensions())
    .build()};
  vk::SurfaceKHR _surface;
  vk::PhysicalDevice _physicalDevice;
  vk::Device _device;
  uint32_t _graphicsQueueIndex;
  uint32_t _presentQueueIndex;
  engine::GameObject::Map _gameObjects;
  vk::DescriptorSetLayout _descriptorSetLayout;
  std::vector<vk::DescriptorSetLayoutBinding> _descriptorSetLayoutBindings;
  vk::PipelineLayout _pipelineLayout;
  vk::Pipeline _pipeline;
  //std::unique_ptr<engine::Renderer> _renderer;

  void init();
  void load();
};
