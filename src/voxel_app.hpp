#pragma once

#include "data/world.hpp"
#include "graphics/world_renderer.hpp"
#include "lve_descriptors.hpp"
#include "lve_device.hpp"
#include "lve_game_object.hpp"
#include "lve_renderer.hpp"
#include "./engine/window.hpp"
#include "./engine/renderer.hpp"
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_vulkan.h>

// std
#include <memory>
#include <vector>

namespace lve {
class VoxelApp {
 public:
  static constexpr std::string APP_NAME = "Voxel App";
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
  LveDevice lveDevice{_window};
  LveRenderer lveRenderer{_window, lveDevice};

  // note: order of declarations matters
  std::unique_ptr<LveDescriptorPool> globalPool{};
  LveGameObject::Map gameObjects;
};
}  // namespace lve
