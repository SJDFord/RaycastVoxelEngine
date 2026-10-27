#pragma once

#include "buffer.hpp"
#include "../vertex.hpp"
#include "../mesh.hpp"
#include "one_time_command_submitter.hpp"

// libs
#include "tinygltf/tiny_gltf_v3.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <memory>
#include <vector>
#include <iostream>

namespace engine {
struct Vertices {
    int count;
    VkBuffer buffer;
    VkDeviceMemory memory;
  };
  struct Indices {
    int count;
    VkBuffer buffer;
    VkDeviceMemory memory;
  };
  

class ModelGltf {
 public:
  ModelGltf(vk::Device device, vk::PhysicalDevice physicalDevice, engine::OneTimeCommandSubmitter& commandSubmitter, const std::string &filepath, float globalscale);
  ~ModelGltf();

  ModelGltf(const ModelGltf &) = delete;
  ModelGltf &operator=(const ModelGltf &) = delete;

  /*
  static std::unique_ptr<Model> createModelFromFile(
      vk::Device device, const std::string &filepath);
*/

  void bind(vk::CommandBuffer commandBuffer);
  void draw(vk::CommandBuffer commandBuffer);

 private:
  void createVertexBuffers(engine::OneTimeCommandSubmitter& commandSubmitter, const std::vector<Vertex> &vertices);
  void createIndexBuffers(engine::OneTimeCommandSubmitter& commandSubmitter, const std::vector<uint32_t> &indices);
  void loadNode(tg3_node* parent, const tg3_node& node, uint32_t nodeIndex, const tg3_model& model, std::vector<uint32_t>& indexBuffer, std::vector<Vertex>& vertexBuffer, float globalscale);
  void loadSkins(tg3_model& gltfModel);
  void loadImages(tg3_model& gltfModel, engine::OneTimeCommandSubmitter& commandSubmitter);
  void loadMaterials(tg3_model& gltfModel);
  void loadAnimations(tg3_model& gltfModel);

  vk::Device _device;
  vk::PhysicalDevice _physicalDevice;
  
  Vertices vertices;

  Indices indices;

  std::vector<tg3_node*> nodes;
  std::vector<tg3_node*> linearNodes;

  std::vector<tg3_skin*> skins;

  std::vector<tg3_texture> textures;
  std::vector<tg3_material> materials;
  std::vector<tg3_animation> animations;
  
  std::unique_ptr<engine::Buffer> vertexBuffer;
  uint32_t vertexCount;

  bool hasIndexBuffer = false;
  std::unique_ptr<engine::Buffer> indexBuffer;
  uint32_t indexCount;
};
}  // namespace lve
