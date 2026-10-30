#pragma once

#include "../one_time_command_submitter.hpp"
#include "../model.hpp"
#include "vulkan/vulkan.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <print>

#include <glm/gtc/type_ptr.hpp>
#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>

namespace engine {

class FileModelBuilder {
 public:
  FileModelBuilder(
    vk::Device const &         device,
    vk::PhysicalDevice const & physicalDevice
  );
  ~FileModelBuilder();

  FileModelBuilder(const FileModelBuilder&) = delete;
  FileModelBuilder& operator=(const FileModelBuilder&) = delete;

  std::unique_ptr<engine::Model> build(OneTimeCommandSubmitter& oneTimeCommandSubmitter, std::string filePath);

 private:
  const vk::Device& _device;
  const vk::PhysicalDevice& _physicalDevice;

  std::vector<Mesh> createMeshes(const fastgltf::Asset& asset, const fastgltf::Mesh& mesh);
  Mesh createMesh(const fastgltf::Asset& asset, const fastgltf::Primitive& primitive);
};
}  // namespace engine