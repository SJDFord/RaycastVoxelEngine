
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include "voxel_app.hpp"
#include "app.hpp"
#include "raii_app.hpp"

// std
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <vector>

// TODO: Look into https://github.com/KhronosGroup/Vulkan-Hpp

int main(int argc, char *argv[]) {
  std::vector<std::string> args(argv, argv+argc);
  std::string appName = "VoxelApp";
  
  if (args.size() > 1) {
        appName = args[1];            
  }

  //lve::VoxelApp app{};
  //App app;
  //RaiiApp app;

  try {
     if (appName == "VoxelApp") {
        std::cout << "Starting " << appName << std::endl;
        lve::VoxelApp app{};
        app.run();
     } else if (appName == "App") {
        std::cout << "Starting " << appName << std::endl;     
        App app{};
        app.run();
     } else if (appName == "RaiiApp") {
        std::cout << "Starting " << appName << std::endl;
        RaiiApp app{};
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
