
#include "voxel_app.hpp"
#include "app.hpp"

// std
#include <cstdlib>
#include <iostream>
#include <stdexcept>

// TODO: Look into https://github.com/KhronosGroup/Vulkan-Hpp

int main() {
  lve::VoxelApp app{};
  //App app;

  try {
    app.run();
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}