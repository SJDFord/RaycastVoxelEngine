#pragma once


// std
#include <memory>
#include <vector>
#include <optional>
#include <iostream>

class RaiiApp {
 public:
  static constexpr int WIDTH = 1920;
  static constexpr int HEIGHT = 1080;

  RaiiApp();
  ~RaiiApp();

  RaiiApp(const RaiiApp &) = delete;
  RaiiApp &operator=(const RaiiApp &) = delete;

  void run();
};
