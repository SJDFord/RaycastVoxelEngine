#pragma once

//#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include "one_time_command_submitter.hpp"
#include "buffer.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <functional>
#include <iostream>
#include <memory>
#include <vector>
namespace engine {
// Encapsulates image and image view/sampling 
// TODO: Separate image creaton and image view/sampling
class Image {
 public:
  // TODO: Other ways to create an image
  Image(
    vk::Device const &         device,
    vk::PhysicalDevice const & physicalDevice,
    OneTimeCommandSubmitter& oneTimeCommandSubmitter,
    const std::string &filepath
  );
  ~Image();

  Image(const Image &) = delete;
  Image &operator=(const Image &) = delete;

  /*
  static std::unique_ptr<Skybox> createSkyboxFromFile(
      lve::LveDevice &device, const std::string &filepath);
      */

  // void bind(VkCommandBuffer commandBuffer);
  // void draw(VkCommandBuffer commandBuffer);
  vk::ImageView getImageView();
  vk::Sampler getSampler();

 private:
  // void createVertexBuffers(const std::vector<Vertex> &vertices);
  // void createIndexBuffers(const std::vector<uint32_t> &indices);
  vk::Device _device;
  vk::PhysicalDevice _physicalDevice;
  vk::Image textureImage = VK_NULL_HANDLE;
  vk::DeviceMemory textureImageMemory = VK_NULL_HANDLE;
  vk::ImageView textureImageView = VK_NULL_HANDLE;
  vk::Sampler textureSampler = VK_NULL_HANDLE;

  void createImage(OneTimeCommandSubmitter& oneTimeCommandSubmitter, const std::string &filepath);
  void createImageView();
  uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
  void transitionImageLayout(
    OneTimeCommandSubmitter& oneTimeCommandSubmitter, vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
  void copyBufferToImage(
    OneTimeCommandSubmitter& oneTimeCommandSubmitter, vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height, uint32_t layerCount);
  /*
  std::unique_ptr<LveBuffer> vertexBuffer;
  uint32_t vertexCount;

  bool hasIndexBuffer = false;
  std::unique_ptr<LveBuffer> indexBuffer;
  uint32_t indexCount;
  */
};

}
