#pragma once

#include "lve_device.hpp"
#include "./engine/buffer.hpp"
#include "graphics/vertex.hpp"
#include "graphics/mesh.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <memory>
#include <vector>
#include <iostream>

namespace lve {
class LveModel {
 public:
  LveModel(vk::Device device, vk::PhysicalDevice physicalDevice, vk::CommandBuffer commandBuffer, const Mesh &mesh);
  ~LveModel();

  LveModel(const LveModel &) = delete;
  LveModel &operator=(const LveModel &) = delete;

  void bind(vk::CommandBuffer commandBuffer);
  void draw(vk::CommandBuffer commandBuffer);

 private:
  void createVertexBuffers(vk::CommandBuffer commandBuffer, const std::vector<Vertex> &vertices);
  void createIndexBuffers(vk::CommandBuffer commandBuffer, const std::vector<uint32_t> &indices);

  vk::Device _device;
  vk::PhysicalDevice _physicalDevice;

  std::unique_ptr<engine::Buffer> vertexBuffer;
  uint32_t vertexCount;

  bool hasIndexBuffer = false;
  std::unique_ptr<engine::Buffer> indexBuffer;
  uint32_t indexCount;
};
}  // namespace lve
