#pragma once

#include "lve_buffer.hpp"
#include "lve_device.hpp"
#include "lve_vertex.hpp"
#include "lve_mesh.hpp"

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
  LveModel(LveDevice &device, const Mesh &mesh);
  ~LveModel();

  LveModel(const LveModel &) = delete;
  LveModel &operator=(const LveModel &) = delete;

  static std::unique_ptr<LveModel> createModelFromFile(
      LveDevice &device, const std::string &filepath);

  void bind(VkCommandBuffer commandBuffer);
  void draw(VkCommandBuffer commandBuffer);

 private:
  void createVertexBuffers(const std::vector<Vertex> &vertices);
  void createIndexBuffers(const std::vector<uint32_t> &indices);

  LveDevice &lveDevice;

  std::unique_ptr<LveBuffer> vertexBuffer;
  uint32_t vertexCount;

  bool hasIndexBuffer = false;
  std::unique_ptr<LveBuffer> indexBuffer;
  uint32_t indexCount;
};
}  // namespace lve
