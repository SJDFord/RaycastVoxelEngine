#include "model.hpp"
//#include "graphics/mesh_builder.hpp"

// std
#include <cassert>
#include <cstring>
#include <unordered_map>

#ifndef ENGINE_DIR
#define ENGINE_DIR "../"
#endif

namespace engine {

Model::Model(vk::Device device, vk::PhysicalDevice physicalDevice, vk::CommandBuffer commandBuffer, const Mesh &mesh) : _device{device}, _physicalDevice{physicalDevice} {
  createVertexBuffers(commandBuffer, mesh.Vertices);
  createIndexBuffers(commandBuffer, mesh.Indices);
}

Model::~Model() {}

/*
std::unique_ptr<LveModel> LveModel::createModelFromFile(
    LveDevice &device, const std::string &filepath) {
  MeshBuilder builder{};
  Mesh mesh = builder.loadMesh(ENGINE_DIR + filepath);
  return std::make_unique<LveModel>(device, mesh);
}
*/

void Model::createVertexBuffers(vk::CommandBuffer commandBuffer, const std::vector<Vertex> &vertices) {
  vertexCount = static_cast<uint32_t>(vertices.size());
  assert(vertexCount >= 3 && "Vertex count must be at least 3");
  vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
  uint32_t vertexSize = sizeof(vertices[0]);

  Buffer stagingBuffer{
      _physicalDevice,
      _device,
      vertexSize,
      //vertexCount,
      vk::BufferUsageFlagBits::eTransferSrc,
      vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
  };

  //stagingBuffer.map();
  stagingBuffer.write(vertices.data());
  
  //stagingBuffer.writeToBuffer((void *)vertices.data());

  vertexBuffer = std::make_unique<Buffer>(
      _physicalDevice,
      _device,
      vertexSize,
      //vertexCount,
      vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
      vk::MemoryPropertyFlagBits::eDeviceLocal
  );


  vk::BufferCopy copyRegion = vk::BufferCopy()
    .setSrcOffset(0)
    .setDstOffset(0)
    .setSize(bufferSize);

  commandBuffer.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), copyRegion);
}

void Model::createIndexBuffers(vk::CommandBuffer commandBuffer, const std::vector<uint32_t> &indices) {
  indexCount = static_cast<uint32_t>(indices.size());
  hasIndexBuffer = indexCount > 0;

  if (!hasIndexBuffer) {
    return;
  }

  vk::DeviceSize bufferSize = sizeof(indices[0]) * indexCount;
  uint32_t indexSize = sizeof(indices[0]);

  Buffer stagingBuffer{
      _physicalDevice,
      _device,
      indexSize,
      //vertexCount,
      vk::BufferUsageFlagBits::eTransferSrc,
      vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
  };

  //stagingBuffer.map();
  //stagingBuffer.writeToBuffer((void *)indices.data());

  stagingBuffer.write(indices.data());

  indexBuffer = std::make_unique<Buffer>(
      _physicalDevice,
      _device,
      indexSize,
      //vertexCount,
      vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
      vk::MemoryPropertyFlagBits::eDeviceLocal
  );
  
  vk::BufferCopy copyRegion = vk::BufferCopy()
    .setSrcOffset(0)
    .setDstOffset(0)
    .setSize(bufferSize);

  commandBuffer.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), copyRegion);
}

void Model::draw(vk::CommandBuffer commandBuffer) {
  if (hasIndexBuffer) {
    commandBuffer.drawIndexed(indexCount, 1, 0, 0, 0);
  } else {
    commandBuffer.draw(vertexCount, 1, 0, 0);
  }
}

void Model::bind(vk::CommandBuffer commandBuffer) {
  vk::Buffer buffers[] = {vertexBuffer->getBuffer()};
  vk::DeviceSize offsets[] = {0};
  commandBuffer.bindVertexBuffers(0, buffers, offsets);

  if (hasIndexBuffer) {
    commandBuffer.bindIndexBuffer(indexBuffer->getBuffer(), 0, vk::IndexType::eUint32);
  }
}

}  // namespace lve
