#pragma once

#include "./engine/window.hpp"
#include "./engine/renderer.hpp"
#include "./engine/descriptor_pool_builder.hpp"

// std
#include <memory>
#include <vector>
#include <optional>
#include <iostream>

class DynamicApp {
 public:

  static constexpr std::string APP_NAME = "Dynamic App";
  static constexpr int WIDTH = 1920;
  static constexpr int HEIGHT = 1080;
  static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

  DynamicApp();
  ~DynamicApp();

  DynamicApp(const DynamicApp &) = delete;
  DynamicApp &operator=(const DynamicApp &) = delete;

  void run();
 private:
  engine::Window _window;
  vk::PhysicalDevice _physicalDevice;
  vk::Device _device;
  uint32_t _graphicsQueueIndex;
  uint32_t _presentQueueIndex;
  //std::unique_ptr<engine::Renderer> _renderer;

  void init();
  void load();
};
