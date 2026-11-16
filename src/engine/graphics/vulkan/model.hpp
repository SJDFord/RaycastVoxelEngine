#pragma once

#include "buffer.hpp"
#include "../vertex.hpp"
#include "../mesh.hpp"
#include "one_time_command_submitter.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <memory>
#include <vector>
#include <iostream>
#include <utility>

namespace engine {
class Model {
 public:
  Model(
    vk::Device device, 
    vk::PhysicalDevice physicalDevice, 
    engine::OneTimeCommandSubmitter& commandSubmitter,
    const std::vector<std::pair<Mesh, int>>& meshTexturePairs
  );
  ~Model();

  Model(const Model &) = delete;
  Model &operator=(const Model &) = delete;

  /*
  static std::unique_ptr<Model> createModelFromFile(
      vk::Device device, const std::string &filepath);
*/

  void bind(vk::CommandBuffer commandBuffer, const vk::PipelineLayout& pipelineLayout);
  void draw(vk::CommandBuffer commandBuffer, const vk::PipelineLayout& pipelineLayout, glm::mat4 modelMatrix, glm::mat4 normalMatrix);
  void setName(std::string name);

 private:
  std::unique_ptr<engine::Buffer> createVertexBuffer(engine::OneTimeCommandSubmitter& commandSubmitter, const std::vector<Vertex> &vertices);
  std::unique_ptr<engine::Buffer> createIndexBuffer(engine::OneTimeCommandSubmitter& commandSubmitter, const std::vector<uint32_t> &indices);

  std::string _name;
  vk::Device _device;
  vk::PhysicalDevice _physicalDevice;

  std::vector<std::unique_ptr<engine::Buffer>> _vertexBuffers;
  std::vector<std::unique_ptr<engine::Buffer>> _indexBuffers;

  struct ModelComponent {
    uint32_t vertexCount;
    uint32_t indexCount;
    int vertexBufferIndex;
    int indexBufferIndex;
    int textureIndex;
  };

  std::vector<ModelComponent> _components;

  /*
  std::unique_ptr<engine::Buffer> vertexBuffer;
  uint32_t vertexCount;

  bool hasIndexBuffer = false;
  std::unique_ptr<engine::Buffer> indexBuffer;
  uint32_t indexCount;
  */
};
}  // namespace lve
