#include "model_gltf.hpp"
//#include "graphics/mesh_builder.hpp"

// std
#include <cassert>
#include <cstring>
#include <unordered_map>

#ifndef ENGINE_DIR
#define ENGINE_DIR "../"
#endif

namespace engine {

ModelGltf::ModelGltf(vk::Device device, vk::PhysicalDevice physicalDevice, engine::OneTimeCommandSubmitter& commandSubmitter, const std::string &filepath, float globalscale) : _device{device}, _physicalDevice{physicalDevice} {

  /*
  createVertexBuffers(commandSubmitter, mesh.Vertices);
  createIndexBuffers(commandSubmitter, mesh.Indices);
  */
}

ModelGltf::~ModelGltf() {}

/*
std::unique_ptr<LveModel> LveModel::createModelFromFile(
    LveDevice &device, const std::string &filepath) {
  MeshBuilder builder{};
  Mesh mesh = builder.loadMesh(ENGINE_DIR + filepath);
  return std::make_unique<LveModel>(device, mesh);
}
*/

void ModelGltf::createVertexBuffers(engine::OneTimeCommandSubmitter& commandSubmitter, const std::vector<Vertex> &vertices) {
  vertexCount = static_cast<uint32_t>(vertices.size());
  assert(vertexCount >= 3 && "Vertex count must be at least 3");
  vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
  uint32_t vertexSize = sizeof(vertices[0]);

  Buffer stagingBuffer{
      _device,
      _physicalDevice,
      vertexSize,
      vertexCount,
      vk::BufferUsageFlagBits::eTransferSrc,
      vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
  };

  stagingBuffer.map();
  stagingBuffer.writeToBuffer((void *)vertices.data());

  vertexBuffer = std::make_unique<Buffer>(
      _device,
      _physicalDevice,
      vertexSize,
      vertexCount,
      vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
      vk::MemoryPropertyFlagBits::eDeviceLocal
  );

  commandSubmitter.submit([&](const vk::CommandBuffer& commandBuffer) {
    stagingBuffer.copyTo(commandBuffer, *vertexBuffer);
  });
}

void ModelGltf::createIndexBuffers(engine::OneTimeCommandSubmitter& commandSubmitter, const std::vector<uint32_t> &indices) {
  indexCount = static_cast<uint32_t>(indices.size());
  hasIndexBuffer = indexCount > 0;

  if (!hasIndexBuffer) {
    return;
  }

  vk::DeviceSize bufferSize = sizeof(indices[0]) * indexCount;
  uint32_t indexSize = sizeof(indices[0]);

  Buffer stagingBuffer{
      _device,
      _physicalDevice,
      indexSize,
      indexCount,
      vk::BufferUsageFlagBits::eTransferSrc,
      vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
  };

  stagingBuffer.map();
  stagingBuffer.writeToBuffer((void *)indices.data());

  //stagingBuffer.write(indices.data());

  indexBuffer = std::make_unique<Buffer>(
      _device,
      _physicalDevice,
      indexSize,
      indexCount,
      vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
      vk::MemoryPropertyFlagBits::eDeviceLocal
  );
  

  commandSubmitter.submit([&](const vk::CommandBuffer& commandBuffer) {
    stagingBuffer.copyTo(commandBuffer, *indexBuffer);
  });
}

void ModelGltf::draw(vk::CommandBuffer commandBuffer) {
  if (hasIndexBuffer) {
    commandBuffer.drawIndexed(indexCount, 1, 0, 0, 0);
  } else {
    commandBuffer.draw(vertexCount, 1, 0, 0);
  }
}

void ModelGltf::bind(vk::CommandBuffer commandBuffer) {
  vertexBuffer->bindVertex(commandBuffer, 0, {0});

  if (hasIndexBuffer) {
    indexBuffer->bindIndex(commandBuffer, 0, vk::IndexType::eUint32);
  }
}

void ModelGltf::loadNode(tg3_node* parent, const tg3_node& node, uint32_t nodeIndex, const tg3_model& model, std::vector<uint32_t>& indexBuffer, std::vector<Vertex>& vertexBuffer, float globalscale) {

}

void ModelGltf::loadSkins(tg3_model& gltfModel) {

}

void ModelGltf::loadImages(tg3_model& gltfModel, engine::OneTimeCommandSubmitter& commandSubmitter) {

}

void ModelGltf::loadMaterials(tg3_model& gltfModel) {

}

void ModelGltf::loadAnimations(tg3_model& gltfModel) {

}

}  // namespace lve
