#include "model.hpp"
//#include "graphics/mesh_builder.hpp"

// std
#include <cassert>
#include <cstring>
#include <unordered_map>
#include <print>

#ifndef ENGINE_DIR
#define ENGINE_DIR "../"
#endif

namespace engine {

Model::Model(vk::Device device, vk::PhysicalDevice physicalDevice, engine::OneTimeCommandSubmitter& commandSubmitter, const std::vector<Mesh>& meshes) : _device{device}, _physicalDevice{physicalDevice} {
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;

  for (auto& mesh : meshes) {
    std::println("Building vertex list");
    vertices.insert(vertices.end(), mesh.Vertices.begin(), mesh.Vertices.end());
    indices.insert(indices.end(), mesh.Indices.begin(), mesh.Indices.end());
  }
  
  createVertexBuffers(commandSubmitter, vertices);
  createIndexBuffers(commandSubmitter, indices);
}

Model::~Model() {}

void Model::createVertexBuffers(engine::OneTimeCommandSubmitter& commandSubmitter, const std::vector<Vertex> &vertices) {
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

void Model::createIndexBuffers(engine::OneTimeCommandSubmitter& commandSubmitter, const std::vector<uint32_t> &indices) {
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

void Model::draw(vk::CommandBuffer commandBuffer) {
  if (hasIndexBuffer) {
    commandBuffer.drawIndexed(indexCount, 1, 0, 0, 0);
  } else {
    commandBuffer.draw(vertexCount, 1, 0, 0);
  }
}

void Model::bind(vk::CommandBuffer commandBuffer) {
  vertexBuffer->bindVertex(commandBuffer, 0, {0});

  if (hasIndexBuffer) {
    indexBuffer->bindIndex(commandBuffer, 0, vk::IndexType::eUint32);
  }
}

}  // namespace lve
