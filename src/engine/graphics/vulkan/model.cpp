#include "model.hpp"
//#include "graphics/mesh_builder.hpp"
#include "../layout/simple.hpp"

// std
#include <cassert>
#include <cstring>
#include <unordered_map>
#include <print>

#ifndef ENGINE_DIR
#define ENGINE_DIR "../"
#endif

namespace engine {

Model::Model(vk::Device device, vk::PhysicalDevice physicalDevice, engine::OneTimeCommandSubmitter& commandSubmitter, 
    const std::vector<std::pair<Mesh, int>>& meshTexturePairs) : _device{device}, _physicalDevice{physicalDevice} {
  //std::vector<Vertex> vertices;
  //std::vector<uint32_t> indices;

  for (auto& meshTexturePair : meshTexturePairs) {
    std::println("Building vertex list");
    //vertices.insert(vertices.end(), mesh.Vertices.begin(), mesh.Vertices.end());
    //indices.insert(indices.end(), mesh.Indices.begin(), mesh.Indices.end());
    ModelComponent component{};
    auto& mesh = meshTexturePair.first;
    std::println("Loading model: {} vertices", mesh.Vertices.size());
    _vertexBuffers.emplace_back(createVertexBuffer(commandSubmitter, mesh.Vertices));
    component.vertexBufferIndex = _vertexBuffers.size() - 1;
    component.vertexCount = mesh.Vertices.size();

    auto indexCount = mesh.Indices.size();
    if (indexCount > 0) {
      _indexBuffers.emplace_back(createIndexBuffer(commandSubmitter, mesh.Indices));
      component.indexBufferIndex = _indexBuffers.size() - 1;
      component.indexCount = indexCount;
    }
    component.textureIndex = meshTexturePair.second;
    _components.push_back(component);
    
    /*
    _components.emplace_back(
      createVertexBuffer(commandSubmitter, mesh.Vertices),
      mesh.Vertices.size(),

    );
    */
    
  }
}

Model::~Model() {}

std::unique_ptr<engine::Buffer> Model::createVertexBuffer(engine::OneTimeCommandSubmitter& commandSubmitter, const std::vector<Vertex> &vertices) {
  auto vertexCount = static_cast<uint32_t>(vertices.size());
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

  auto vertexBuffer = std::make_unique<Buffer>(
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

  return vertexBuffer;
}

std::unique_ptr<engine::Buffer> Model::createIndexBuffer(engine::OneTimeCommandSubmitter& commandSubmitter, const std::vector<uint32_t> &indices) {
  auto indexCount = static_cast<uint32_t>(indices.size());

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

  auto indexBuffer = std::make_unique<Buffer>(
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

  return indexBuffer;
}

void Model::draw(vk::CommandBuffer commandBuffer, const vk::PipelineLayout& pipelineLayout, glm::mat4 modelMatrix, glm::mat4 normalMatrix) {
  for (auto& component : _components) {
    _vertexBuffers[component.vertexBufferIndex]->bindVertex(commandBuffer, 0, {0});

    if (component.indexCount > 0) {
      _indexBuffers[component.indexBufferIndex]->bindIndex(commandBuffer, 0, vk::IndexType::eUint32);
    }

    engine::SimplePushConstantData push{};
    push.modelMatrix = modelMatrix;
    push.normalMatrix = normalMatrix;
    push.textureIndex = component.textureIndex;


    commandBuffer.pushConstants(
      pipelineLayout, 
      vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 
      0,
      sizeof(engine::SimplePushConstantData),
      &push);

    if (component.indexCount > 0) {
      commandBuffer.drawIndexed(component.indexCount, 1, 0, 0, 0);
    } else {
      commandBuffer.draw(component.vertexCount, 1, 0, 0);
    }
  }
}

void Model::bind(vk::CommandBuffer commandBuffer, const vk::PipelineLayout& pipelineLayout) {
  /*
  engine::PerModelPushConstantData push{};
  push.textureIndex = 0;
   
  commandBuffer.pushConstants(
    pipelineLayout, 
    vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 
    0,
    sizeof(engine::PerModelPushConstantData),
    &push);
  */

  /*
  vertexBuffer->bindVertex(commandBuffer, 0, {0});

  if (hasIndexBuffer) {
    indexBuffer->bindIndex(commandBuffer, 0, vk::IndexType::eUint32);
  }
  */
}

}  // namespace lve
