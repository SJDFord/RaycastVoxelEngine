#pragma once

// #define STB_ImageRaii_IMPLEMENTATION
#include "graphics/mesh.hpp"
#include "graphics/vertex.hpp"
#include "buffer.hpp"
#include "device.hpp"
#include "stb/stb_image.h"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <iostream>
#include <lve_device.hpp>
#include <memory>
#include <vector>

// Encapsulates ImageRaii and ImageRaii view/sampling
// TODO: Separate ImageRaii creaton and ImageRaii view/sampling
class ImageRaii {
 public:
  // TODO: Other ways to create an ImageRaii
  ImageRaii(std::shared_ptr<Device> device, const std::string &filepath);
  ~ImageRaii();

  ImageRaii(const ImageRaii &) = delete;
  ImageRaii &operator=(const ImageRaii &) = delete;

  /*
  static std::unique_ptr<Skybox> createSkyboxFromFile(
      lve::LveDevice &device, const std::string &filepath);
      */

  // void bind(VkCommandBuffer commandBuffer);
  // void draw(VkCommandBuffer commandBuffer);
  vk::raii::ImageView& getImageView();
  vk::raii::Sampler& getSampler();

 private:
  // void createVertexBuffers(const std::vector<Vertex> &vertices);
  // void createIndexBuffers(const std::vector<uint32_t> &indices);

  std::shared_ptr<Device> device;
  std::shared_ptr<vk::raii::Image> textureImage = VK_NULL_HANDLE;
  std::shared_ptr<vk::raii::DeviceMemory> textureImageMemory = VK_NULL_HANDLE;
  vk::raii::ImageView textureImageView = VK_NULL_HANDLE;
  vk::raii::Sampler textureSampler = VK_NULL_HANDLE;

  void createImage();
  void createImageView();
  static void loadCubemap(std::shared_ptr<Device> device, std::string filename, vk::Format format);
  /*
  std::unique_ptr<LveBuffer> vertexBuffer;
  uint32_t vertexCount;

  bool hasIndexBuffer = false;
  std::unique_ptr<LveBuffer> indexBuffer;
  uint32_t indexCount;
  */
};
