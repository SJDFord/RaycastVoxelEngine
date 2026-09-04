
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include "voxel_app.hpp"
#include "app.hpp"
#include "raii_app.hpp"
#include "dynamic_app.hpp"

// std
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <vector>

// TODO: Look into https://github.com/KhronosGroup/Vulkan-Hpp

int main(int argc, char *argv[]) {
  std::vector<std::string> args(argv, argv+argc);
  std::string appName = "RaiiApp";
  bool isDynamic = false;
  
  if (args.size() > 1) {
        appName = args[1];            
  }

  if (args.size() > 2) {
      isDynamic = args[2] == "DYNAMIC";
  }

  try {
      std::cout << "Starting " << appName << std::endl;
     if (appName == "VoxelApp") {
        lve::VoxelApp app{};
        app.run();
     } else if (appName == "App") {
        App app{};
        app.run();
     } else if (appName == "RaiiApp") {
        RaiiApp app(isDynamic);
        app.run();
     } else if (appName == "DynamicApp") {
        DynamicApp app{};
        app.run();
     } else {
        std::cerr << "App name not recognised: " << appName << std::endl;
     }
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
