#pragma once

#include "../one_time_command_submitter.hpp"
#include "../model.hpp"
#include "../image.hpp"
#include "../image_registry.hpp"
#include <map>
#include "vulkan/vulkan.hpp"
#include "file_image_builder.hpp"
#include <filesystem>



#include "stb/stb_image.h"
#include <iostream>
#include <string>
#include <vector>
#include <print>
#include <utility>

#include <glm/gtc/type_ptr.hpp>
#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>
#include <algorithm>

#include <png.h>

namespace engine {
  typedef struct {
const png_byte* data;
const png_size_t size;
} DataHandle;
typedef struct {
const DataHandle data;
png_size_t offset;
} ReadDataHandle;

#define PNG_SIG_BYTES (8) /* bytes in the PNG file signature. */

#define PNG_RGBA_PIXEL_LIMIT (0x1000000)


class FileModelBuilder {
 public:
  FileModelBuilder(
    vk::Device const &         device,
    vk::PhysicalDevice const & physicalDevice
  );
  ~FileModelBuilder();

  FileModelBuilder(const FileModelBuilder&) = delete;
  FileModelBuilder& operator=(const FileModelBuilder&) = delete;

  std::unique_ptr<engine::Model> build(OneTimeCommandSubmitter& oneTimeCommandSubmitter, std::string filePath, ImageRegistry& imageRegistry);

 private:
  const vk::Device& _device;
  const vk::PhysicalDevice& _physicalDevice;
  std::string _modelName;
  std::string _directory;
  int _imageIndex = 0;


  std::vector<ModelMesh> createMeshes(const fastgltf::Asset& asset, fastgltf::math::fmat4x4 nodeMatrix, const fastgltf::Mesh& mesh, OneTimeCommandSubmitter& oneTimeCommandSubmitter);
  ModelMesh createMesh(const fastgltf::Asset& asset, fastgltf::math::fmat4x4 nodeMatrix, const fastgltf::Mesh& mesh, const fastgltf::Primitive& primitive, OneTimeCommandSubmitter& oneTimeCommandSubmitter);
  int createImage(const fastgltf::Asset& asset, const fastgltf::Image& image, OneTimeCommandSubmitter& oneTimeCommandSubmitter, ImageRegistry& imageRegistry);
  int createImage( 
    fastgltf::sources::Array& array, 
    OneTimeCommandSubmitter& oneTimeCommandSubmitter, 
    ImageRegistry& imageRegistry
  );
  std::string toString(fastgltf::AccessorType accessorType);
  std::string toString(fastgltf::ComponentType componentType);
  std::string toString(fastgltf::MimeType mimeType);

  int readPngStb(std::byte *data, std::size_t size, OneTimeCommandSubmitter& oneTimeCommandSubmitter, ImageRegistry& imageRegistry);
  int readPngLibpng(std::byte *data, std::size_t size, OneTimeCommandSubmitter& oneTimeCommandSubmitter, ImageRegistry& imageRegistry);
};
}  // namespace engine