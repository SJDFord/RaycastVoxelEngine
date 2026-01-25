#pragma once

// std
#include <memory>
#include <vector>
#include <optional>

class App {
 public:
  static constexpr int WIDTH = 1920;
  static constexpr int HEIGHT = 1080;

  App();
  ~App();

  App(const App &) = delete;
  App &operator=(const App &) = delete;

  void run();
};
