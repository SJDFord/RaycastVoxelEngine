#pragma once

#include "data/world.hpp"
#include "graphics/world_renderer.hpp"
#include "lve_descriptors.hpp"
#include "lve_device.hpp"
#include "lve_game_object.hpp"
#include "lve_renderer.hpp"
#include "lve_window.hpp"
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_vulkan.h>

// std
#include <memory>
#include <vector>

namespace lve {
class VoxelApp {
 public:
  static constexpr int WIDTH = 1920;
  static constexpr int HEIGHT = 1080;

  VoxelApp();
  ~VoxelApp();

  VoxelApp(const VoxelApp &) = delete;
  VoxelApp &operator=(const VoxelApp &) = delete;

  void run();

 private:
  void loadGameObjects();

  LveWindow lveWindow{WIDTH, HEIGHT, "Vulkan Tutorial"};
  LveDevice lveDevice{lveWindow};
  LveRenderer lveRenderer{lveWindow, lveDevice};
  std::shared_ptr<World> _world;
  std::unique_ptr<WorldRenderer> _worldRenderer;

  // note: order of declarations matters
  std::unique_ptr<LveDescriptorPool> globalPool{};
  LveGameObject::Map gameObjects;
};
}  // namespace lve
