#pragma once

#include "buffer.hpp"
#include "vertex.hpp"
#include "mesh.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <memory>
#include <vector>
#include <iostream>

namespace engine {
class Model {
 public:
  Model(vk::Device device, vk::PhysicalDevice physicalDevice, vk::CommandBuffer commandBuffer, const Mesh &mesh);
  ~Model();

  Model(const Model &) = delete;
  Model &operator=(const Model &) = delete;

  /*
  static std::unique_ptr<Model> createModelFromFile(
      vk::Device device, const std::string &filepath);
*/

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
