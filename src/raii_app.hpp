#pragma once

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_vulkan.h>

#include "data/world.hpp"
#include "graphics/world_renderer.hpp"
#include "descriptors.hpp"
#include "device.hpp"
#include "window.hpp"
#include "game_object.hpp"

// std
#include <memory>
#include <vector>

namespace lve {
class RaiiApp {
 public:
  static constexpr int WIDTH = 1920;
  static constexpr int HEIGHT = 1080;

  RaiiApp();
  ~RaiiApp();

  RaiiApp(const RaiiApp &) = delete;
  RaiiApp &operator=(const RaiiApp &) = delete;

  void run();

 private:
  void loadGameObjects();

  std::shared_ptr<Window> window;
   //{WIDTH, HEIGHT, "Vulkan RAII Voxel Engine"};
  std::shared_ptr<Device> device;
  //{window};
  /*
  LveRenderer lveRenderer{lveWindow, lveDevice};
  */
  // note: order of declarations matters
  std::unique_ptr<DescriptorPool> globalPool{};
  GameObject::Map gameObjects;
};
}  // namespace lve
