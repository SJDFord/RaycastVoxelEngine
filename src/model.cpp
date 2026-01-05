#include "graphics/mesh_builder.hpp"
#include "model.hpp"

// std
#include <cassert>
#include <cstring>
#include <unordered_map>

#ifndef ENGINE_DIR
#define ENGINE_DIR "../"
#endif


Model::Model(std::shared_ptr<Device> device, const Mesh &mesh) : _device{device} {
  createVertexBuffers(mesh.Vertices);
  createIndexBuffers(mesh.Indices);
}

Model::~Model() {}

std::unique_ptr<Model> Model::createModelFromFile(
    std::shared_ptr<Device> device, const std::string &filepath) {
  MeshBuilder builder{};
  Mesh mesh = builder.loadMesh(ENGINE_DIR + filepath);
  return std::make_unique<Model>(device, mesh);
}

void Model::createVertexBuffers(const std::vector<Vertex> &vertices) {
  vertexCount = static_cast<uint32_t>(vertices.size());
  assert(vertexCount >= 3 && "Vertex count must be at least 3");
  vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
  uint32_t vertexSize = sizeof(vertices[0]);

  Buffer stagingBuffer {
      _device,
      vertexSize,
      vertexCount,
      vk::BufferUsageFlagBits::eTransferSrc,
      vk::MemoryPropertyFlagBits::eHostVisible |
      vk::MemoryPropertyFlagBits::eHostCoherent
  };

  stagingBuffer.map();
  stagingBuffer.writeToBuffer((void *)vertices.data());

  vertexBuffer = std::make_unique<Buffer>(
      _device,
      vertexSize,
      vertexCount,
      vk::BufferUsageFlagBits::eVertexBuffer |
      vk::BufferUsageFlagBits::eTransferDst,
      vk::MemoryPropertyFlagBits::eDeviceLocal
  );

  _device->copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
}

void Model::createIndexBuffers(const std::vector<uint32_t> &indices) {
  indexCount = static_cast<uint32_t>(indices.size());
  hasIndexBuffer = indexCount > 0;

  if (!hasIndexBuffer) {
    return;
  }

  vk::DeviceSize bufferSize = sizeof(indices[0]) * indexCount;
  uint32_t indexSize = sizeof(indices[0]);

  Buffer stagingBuffer{
      _device,
      indexSize,
      indexCount,
      vk::BufferUsageFlagBits::eTransferSrc,
      vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
  };

  stagingBuffer.map();
  stagingBuffer.writeToBuffer((void *)indices.data());

  indexBuffer = std::make_unique<Buffer>(
      _device,
      indexSize,
      indexCount,
      vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
      vk::MemoryPropertyFlagBits::eDeviceLocal);

  _device->copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);
}

void Model::draw(const vk::raii::CommandBuffer& commandBuffer) {
  if (hasIndexBuffer) {
    commandBuffer.drawIndexed(indexCount, 1, 0, 0, 0);
  } else {
    commandBuffer.draw(vertexCount, 1, 0, 0);
  }
}

void Model::bind(const vk::raii::CommandBuffer& commandBuffer) {
  vk::DeviceSize offsets[] = {0};  
  commandBuffer.bindVertexBuffers(0, *(vertexBuffer->getBuffer()), offsets);

  if (hasIndexBuffer) {
    commandBuffer.bindIndexBuffer(indexBuffer->getBuffer(), 0, vk::IndexType::eUint32);
  }
}
