#pragma once

//#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"


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
#include <lve_device.hpp>
#include <memory>
#include <vector>

// Encapsulates image and image view/sampling 
// TODO: Separate image creaton and image view/sampling
class Image {
 public:
  // TODO: Other ways to create an image
  Image(lve::LveDevice &device, const std::string &filepath);
  ~Image();

  Image(const Image &) = delete;
  Image &operator=(const Image &) = delete;

  /*
  static std::unique_ptr<Skybox> createSkyboxFromFile(
      lve::LveDevice &device, const std::string &filepath);
      */

  // void bind(VkCommandBuffer commandBuffer);
  // void draw(VkCommandBuffer commandBuffer);
  VkImageView getImageView();
  VkSampler getSampler();

 private:
  // void createVertexBuffers(const std::vector<Vertex> &vertices);
  // void createIndexBuffers(const std::vector<uint32_t> &indices);

  lve::LveDevice &lveDevice;
  VkImage textureImage = VK_NULL_HANDLE;
  VkDeviceMemory textureImageMemory = VK_NULL_HANDLE;
  VkImageView textureImageView = VK_NULL_HANDLE;
  VkSampler textureSampler = VK_NULL_HANDLE;

  void createImage();
  void createImageView();
  static void loadCubemap(lve::LveDevice &device, std::string filename, VkFormat format);
  /*
  std::unique_ptr<LveBuffer> vertexBuffer;
  uint32_t vertexCount;

  bool hasIndexBuffer = false;
  std::unique_ptr<LveBuffer> indexBuffer;
  uint32_t indexCount;
  */
};
