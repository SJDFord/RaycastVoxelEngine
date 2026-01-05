#pragma once

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
#include <device.hpp>
#include <buffer.hpp>

class Model {
 public:
  Model(std::shared_ptr<Device> device, const Mesh &mesh);
  ~Model();

  Model(const Model &) = delete;
  Model &operator=(const Model &) = delete;

  static std::unique_ptr<Model> createModelFromFile(
      std::shared_ptr<Device> device, const std::string &filepath);

  void bind(const vk::raii::CommandBuffer& commandBuffer);
  void draw(const vk::raii::CommandBuffer& commandBuffer);

 private:
  void createVertexBuffers(const std::vector<Vertex> &vertices);
  void createIndexBuffers(const std::vector<uint32_t> &indices);

  std::shared_ptr<Device> _device;

  std::unique_ptr<Buffer> vertexBuffer;
  uint32_t vertexCount;

  bool hasIndexBuffer = false;
  std::unique_ptr<Buffer> indexBuffer;
  uint32_t indexCount;
};
