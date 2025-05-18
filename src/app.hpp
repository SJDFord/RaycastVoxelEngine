#pragma once

#include "lve_descriptors.hpp"
#include "game_object.hpp"
#include "lve_renderer.hpp"
#include "window.hpp"
#include "data/world.hpp"
#include "graphics/world_renderer.hpp"

// std
#include <memory>
#include <vector>
#include <set>
#include <device.hpp>

class App {
 public:
  static constexpr int WIDTH = 1920;
  static constexpr int HEIGHT = 1080;

  App();
  ~App();

  App(const App &) = delete;
  App &operator=(const App &) = delete;

  void run();

 private:
  void setup();

  Window _window{WIDTH, HEIGHT, "Vulkan Tutorial"};
  Device _device{_window};

  GameObject::Map gameObjects;

    
  const std::vector<const char *> _validationLayers = {"VK_LAYER_KHRONOS_validation"};
  const std::vector<const char *> _deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
};