#pragma once

#include "window.hpp"
#include "device.hpp"

// std
#include <memory>
#include <vector>
#include <optional>

constexpr uint32_t IN_FLIGHT_FRAME_COUNT{2};

struct Frame {
  vk::raii::CommandBuffer commandBuffer;
  vk::raii::Semaphore imageAvailableSemaphore;
  vk::raii::Semaphore renderFinishedSemaphore;
  vk::raii::Fence fence;
};

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
  std::shared_ptr<Window> window;
  std::shared_ptr<Device> device;
  std::array<std::optional<Frame>, IN_FLIGHT_FRAME_COUNT> frames{};
  uint32_t frameIndex{};

  void initFrames();
};