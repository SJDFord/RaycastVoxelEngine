#include "lve_model.hpp"
#include "graphics/mesh_builder.hpp"

// std
#include <cassert>
#include <cstring>
#include <unordered_map>

#ifndef ENGINE_DIR
#define ENGINE_DIR "../"
#endif

namespace lve {

LveModel::LveModel(LveDevice &device, const Mesh &mesh) : lveDevice{device} {
  createVertexBuffers(mesh.Vertices);
  createIndexBuffers(mesh.Indices);
}

LveModel::~LveModel() {}

std::unique_ptr<LveModel> LveModel::createModelFromFile(
    LveDevice &device, const std::string &filepath) {
  MeshBuilder builder{};
  Mesh mesh = builder.loadMesh(ENGINE_DIR + filepath);
  return std::make_unique<LveModel>(device, mesh);
}

void LveModel::createVertexBuffers(const std::vector<Vertex> &vertices) {
  vertexCount = static_cast<uint32_t>(vertices.size());
  assert(vertexCount >= 3 && "Vertex count must be at least 3");
  vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
  uint32_t vertexSize = sizeof(vertices[0]);

  engine::Buffer stagingBuffer{
      lveDevice.device(),
      lveDevice.getPhysicalDevice(),
      vertexSize,
      vertexCount,
      vk::BufferUsageFlagBits::eTransferSrc,
      vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
  };

  stagingBuffer.map();
  stagingBuffer.writeToBuffer((void *)vertices.data());

  vertexBuffer = std::make_unique<engine::Buffer>(
      lveDevice.device(),
      lveDevice.getPhysicalDevice(),
      vertexSize,
      vertexCount,
      vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
      vk::MemoryPropertyFlagBits::eDeviceLocal);

  lveDevice.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
}

void LveModel::createIndexBuffers(const std::vector<uint32_t> &indices) {
  indexCount = static_cast<uint32_t>(indices.size());
  hasIndexBuffer = indexCount > 0;

  if (!hasIndexBuffer) {
    return;
  }

  vk::DeviceSize bufferSize = sizeof(indices[0]) * indexCount;
  uint32_t indexSize = sizeof(indices[0]);

  engine::Buffer stagingBuffer{
      lveDevice.device(),
      lveDevice.getPhysicalDevice(),
      indexSize,
      indexCount,
      vk::BufferUsageFlagBits::eTransferSrc,
      vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
  };

  stagingBuffer.map();
  stagingBuffer.writeToBuffer((void *)indices.data());

  indexBuffer = std::make_unique<engine::Buffer>(
      lveDevice.device(),
      lveDevice.getPhysicalDevice(),
      indexSize,
      indexCount,
      vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
      vk::MemoryPropertyFlagBits::eDeviceLocal);

  lveDevice.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);
}

void LveModel::draw(vk::CommandBuffer commandBuffer) {
  if (hasIndexBuffer) {
    vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
  } else {
    vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
  }
}

void LveModel::bind(vk::CommandBuffer commandBuffer) {
  VkBuffer buffers[] = {vertexBuffer->getBuffer()};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

  if (hasIndexBuffer) {
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
  }
}

}  // namespace lve
