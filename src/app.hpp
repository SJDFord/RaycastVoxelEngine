#pragma once

#include "data/world.hpp"
#include "graphics/world_renderer.hpp"
#include "lve_descriptors.hpp"
#include "lve_device.hpp"
#include "lve_game_object.hpp"
#include "lve_model.hpp"
#include "lve_renderer.hpp"
#include "lve_window.hpp"
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_vulkan.h>

// std
#include <memory>
#include <vector>

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
  void loadGameObjects();

  lve::LveWindow lveWindow{WIDTH, HEIGHT, "Vulkan Voxel Engine"};
  lve::LveDevice lveDevice{lveWindow};
  lve::LveRenderer lveRenderer{lveWindow, lveDevice};
  std::shared_ptr<World> _world;
  std::unique_ptr<WorldRenderer> _worldRenderer;

  // note: order of declarations matters
  std::unique_ptr<lve::LveDescriptorPool> globalPool{};
  lve::LveGameObject::Map gameObjects;
};