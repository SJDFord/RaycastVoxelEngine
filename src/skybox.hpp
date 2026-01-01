#pragma once

//#define STB_IMAGE_IMPLEMENTATION
//#include "stb/stb_image.h"


#include "graphics/mesh.hpp"
#include "graphics/vertex.hpp"
#include "lve_buffer.hpp"
#include "lve_device.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <iostream>
#include <memory>
#include <vector>
#include <lve_device.hpp>

class Skybox {
 public:
     // TODO: Other ways to create skybox
  Skybox(lve::LveDevice &device, const std::string &filepath);
  ~Skybox();

  Skybox(const Skybox &) = delete;
  Skybox &operator=(const Skybox &) = delete;

  /*
  static std::unique_ptr<Skybox> createSkyboxFromFile(
      lve::LveDevice &device, const std::string &filepath);
      */

  //void bind(VkCommandBuffer commandBuffer);
  //void draw(VkCommandBuffer commandBuffer);

 private:
  //void createVertexBuffers(const std::vector<Vertex> &vertices);
  //void createIndexBuffers(const std::vector<uint32_t> &indices);

  lve::LveDevice &lveDevice;
  VkImage textureImage = VK_NULL_HANDLE;
  VkDeviceMemory textureImageMemory = VK_NULL_HANDLE;

  static void loadCubemap(lve::LveDevice &device, std::string filename, VkFormat format);

  /*
  std::unique_ptr<LveBuffer> vertexBuffer;
  uint32_t vertexCount;

  bool hasIndexBuffer = false;
  std::unique_ptr<LveBuffer> indexBuffer;
  uint32_t indexCount;
  */
};
