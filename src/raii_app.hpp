#pragma once

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_vulkan.h>

#include "data/world.hpp"
#include "graphics/world_renderer.hpp"
#include "device.hpp"
#include "window.hpp"
#include "game_object.hpp"
#include "renderer.hpp"

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
  void createDescriptorPool();
  void createDescriptorSetLayout();

  std::shared_ptr<Window> window;
  std::shared_ptr<Device> device;
  std::shared_ptr<Renderer> renderer;

  // note: order of declarations matters
  std::shared_ptr<vk::raii::DescriptorPool> globalPool{};
  std::vector<vk::DescriptorSetLayoutBinding> setLayoutBindings;
  std::shared_ptr<vk::raii::DescriptorSetLayout> descriptorSetLayout;
  GameObject::Map gameObjects;
};
}  // namespace lve
