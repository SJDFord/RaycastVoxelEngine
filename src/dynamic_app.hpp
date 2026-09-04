#pragma once


// std
#include <memory>
#include <vector>
#include <optional>
#include <iostream>

class DynamicApp {
 public:
  static constexpr int WIDTH = 1920;
  static constexpr int HEIGHT = 1080;
  static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

  DynamicApp();
  ~DynamicApp();

  DynamicApp(const DynamicApp &) = delete;
  DynamicApp &operator=(const DynamicApp &) = delete;

  void run();
 private:
  void load();
};
