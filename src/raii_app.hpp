#pragma once

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_vulkan.h>

#include "data/world.hpp"
#include "graphics/world_renderer.hpp"
#include "lve_descriptors.hpp"
#include "device.hpp"
#include "lve_game_object.hpp"
#include "lve_renderer.hpp"
#include "window.hpp"

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

  Window window{WIDTH, HEIGHT, "Vulkan RAII Voxel Engine"};
  Device device{window};
  /*
  LveRenderer lveRenderer{lveWindow, lveDevice};
  std::shared_ptr<World> _world;
  std::unique_ptr<WorldRenderer> _worldRenderer;

  // note: order of declarations matters
  std::unique_ptr<LveDescriptorPool> globalPool{};
  LveGameObject::Map gameObjects;
  */
};
}  // namespace lve
