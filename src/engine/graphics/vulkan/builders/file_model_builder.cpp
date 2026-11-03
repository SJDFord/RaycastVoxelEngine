#include "file_model_builder.hpp"

namespace engine {

FileModelBuilder::FileModelBuilder(
    vk::Device const &         device,
    vk::PhysicalDevice const & physicalDevice
  ) : _device{device}, _physicalDevice{physicalDevice} {

}

FileModelBuilder::~FileModelBuilder() {
    
}


std::unique_ptr<engine::Model> FileModelBuilder::build(OneTimeCommandSubmitter& oneTimeCommandSubmitter, std::string filePath) {
    if (!filePath.ends_with(".gltf")) {
        throw std::format_error("File types other than GLTF are not supported");
    }

    fastgltf::Parser parser;

    auto data = fastgltf::GltfDataBuffer::FromPath(filePath);
    if (data.error() != fastgltf::Error::None) {
      throw std::format_error("Could not parse gltf file");
    }

    // This loads the glTF file into the gltf object and parses the JSON.
    // It automatically detects whether this is a JSON-based or binary glTF.
    // If you know the type, you can also use loadGltfJson or loadGltfBinary.
    auto lastSlashIndex = filePath.find_last_of("/");
    auto directory = filePath.substr(0, lastSlashIndex);
    _modelName = filePath.substr(lastSlashIndex + 1);
    auto assetResult = parser.loadGltf(data.get(), directory, fastgltf::Options::None);
    if (auto error = assetResult.error(); error != fastgltf::Error::None) {
      throw std::format_error("Could not load gltf file");
    }

    fastgltf::Asset& asset = assetResult.get();
    auto initialSceneIndex = asset.defaultScene.value_or(0);
    auto& scene = asset.scenes[initialSceneIndex];

    auto initial = fastgltf::math::fmat4x4();

    std::vector<std::vector<Mesh>> meshes;
    std::println("Model {}: {} meshes", _modelName, asset.meshes.size());
    meshes.resize(asset.meshes.size());

    auto processNode = [&](std::size_t nodeIndex, fastgltf::math::fmat4x4 nodeMatrix, auto& self) -> void {
      std::println("Model {}: {} nodes", _modelName, asset.nodes.size());
      assert(asset.nodes.size() > nodeIndex);
      auto& node = asset.nodes[nodeIndex];
      nodeMatrix = fastgltf::getTransformMatrix(node, nodeMatrix);

      if (node.meshIndex.has_value()) {
          std::size_t meshIndex = node.meshIndex.value();
          fastgltf::Mesh& mesh = asset.meshes[meshIndex];
          std::vector<Mesh> engineMeshes = createMeshes(asset, mesh);
          meshes.push_back(engineMeshes);
      }

      for (auto& child : node.children) {
        self(child, nodeMatrix, self);
      }
    };

    for (auto& sceneNode : scene.nodeIndices) {
      processNode(sceneNode, initial, processNode);
    }

    std::vector<Mesh> flattenedMeshes;

    for (auto& meshList : meshes) {
      flattenedMeshes.insert(flattenedMeshes.end(), meshList.begin(), meshList.end());
    }
  
    return std::make_unique<engine::Model>(_device, _physicalDevice, oneTimeCommandSubmitter, flattenedMeshes);
}

std::vector<Mesh> FileModelBuilder::createMeshes(const fastgltf::Asset& asset, const fastgltf::Mesh& mesh) {
  auto& primitives = mesh.primitives;
  std::println("Model {}: Mesh {}: {} primitives, {} textures, {} images, {} materials", _modelName, mesh.name, primitives.size(), asset.textures.size(), asset.images.size(), asset.materials.size());

  std::vector<Mesh> meshes;
  meshes.resize(primitives.size());
  
  for (const fastgltf::Primitive& primitive : primitives) {
    meshes.push_back(createMesh(asset, mesh, primitive));
  }

  return meshes;
}


Mesh FileModelBuilder::createMesh(const fastgltf::Asset& asset, const fastgltf::Mesh& mesh, const fastgltf::Primitive& primitive) {
  std::println("Model {}: Mesh {}: Primitive: {}", _modelName, mesh.name, "");

  std::vector<Vertex> vertices;
  if (primitive.type != fastgltf::PrimitiveType::Triangles) {
    throw std::format_error("Encountered unsupported primitive type");
  }

  const fastgltf::Attribute* positionAttribute = primitive.findAttribute("POSITION");
  if (!positionAttribute) {
    throw std::format_error("Missing position attribute");
  }

  for (auto& image : asset.images) {
    if (std::holds_alternative<fastgltf::sources::URI>(image.data)) {
        const auto& uri = std::get<fastgltf::sources::URI>(image.data);
        std::println("Model {}: URI: {}", _modelName, uri.uri.c_str());      
        // Handle external URI/file path
    } else if (std::holds_alternative<fastgltf::sources::Vector>(image.data)) {
        const auto& vectorSource = std::get<fastgltf::sources::Vector>(image.data);
        auto mimeType = toString(vectorSource.mimeType);
        std::println("Model {}: Vector ({})", _modelName, mimeType);
        // vectorSource.bytes contains the raw encoded file data (PNG/JPG/etc.)
    } else if (std::holds_alternative<fastgltf::sources::BufferView>(image.data)) {
        const auto& bufferViewSource = std::get<fastgltf::sources::BufferView>(image.data);
        auto mimeType = toString(bufferViewSource.mimeType);
        std::println("Model {}: Buffer View ({})", _modelName, mimeType);
          /*
          std::unique_ptr<engine::Image> image = engine::FileImageBuilder(
            _device, 
            _physicalDevice
          ).build(
            _oneTimeCommandSubmitter, 
            //"../textures/hinoki_planks_diff_4k.jpg"
            "../textures/metalplate01_rgba.ktx"
          );
          */
        // Access asset.bufferViews[bufferViewSource.bufferViewIndex]
    }
  }

  auto& positionAccessor = asset.accessors[positionAttribute->accessorIndex];
  std::println("Model {}: Mesh {}: Primitive: {}: Vertices: {}", _modelName, mesh.name, "", positionAccessor.count);
  vertices.resize(positionAccessor.count);
  fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(asset, positionAccessor, [&](fastgltf::math::fvec3 pos, std::size_t idx) {
    glm::vec3 position = glm::vec3(pos.x(), pos.y(), pos.z());
    vertices[idx].position = position;
  });

  const fastgltf::Attribute* normalAttribute = primitive.findAttribute("NORMAL");
  if (!normalAttribute) {
    throw std::format_error("Missing normal attribute");
  }

  auto& normalAccessor = asset.accessors[normalAttribute->accessorIndex];
  std::println("Model {}: Mesh {}: Primitive: {}: Normals: {}", _modelName, mesh.name, "", normalAccessor.count);
  fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(asset, normalAccessor, [&](fastgltf::math::fvec3 pos, std::size_t idx) {
    //std::println("Adding normal {}", idx);
    vertices[idx].normal = glm::vec3(pos.x(), pos.y(), pos.z());
  });

  const fastgltf::Attribute* textCoordAttribute = primitive.findAttribute("TEXCOORD_0");

  if (!textCoordAttribute) {
    throw std::format_error("Missing normal attribute");
  }

  auto& texCoordAccessor = asset.accessors[textCoordAttribute->accessorIndex];
  std::println("Model {}: Mesh {}: Primitive: {}: Tex Coords: {} {} {}", _modelName, mesh.name, "", texCoordAccessor.count, toString(texCoordAccessor.type), toString(texCoordAccessor.componentType));
  
  if (texCoordAccessor.componentType != fastgltf::ComponentType::Float) {
    throw std::format_error("Component type not supported");
  }

  switch (texCoordAccessor.type) {
    case fastgltf::AccessorType::Vec2:
      fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec2>(asset, texCoordAccessor, [&](fastgltf::math::fvec2 uv, std::size_t idx) {
        
          //std::println("Adding uv {}", idx);
          vertices[idx].uvw = glm::vec3(uv.x(), uv.y(), 0.0f);
      });
      break;
    case fastgltf::AccessorType::Vec3:
      fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(asset, texCoordAccessor, [&](fastgltf::math::fvec3 uv, std::size_t idx) {
        
          //std::println("Adding uv {}", idx);
          vertices[idx].uvw = glm::vec3(uv.x(), uv.y(), uv.z());
      });
      break;
    default:
      throw std::format_error("Accessor type not supported");
  }

  std::println("Reading indices");

  std::vector<std::uint32_t> indices;
  if (primitive.indicesAccessor.has_value()) {
    auto& accessor = asset.accessors[primitive.indicesAccessor.value()];
    std::println("Model {}: Mesh {}: Primitive: {}: Indices: {}", _modelName, mesh.name, "", accessor.count);
    indices.resize(accessor.count);

    if (accessor.type == fastgltf::AccessorType::Scalar) {
      std::println("Scalar accessor type");
    }
    if (accessor.componentType == fastgltf::ComponentType::Int) {
      std::println("Uint accessor component type");
    }

    fastgltf::iterateAccessorWithIndex<std::uint32_t>(asset, accessor, [&](std::uint32_t index, std::size_t idx) {
        indices[idx] = index;
    });
  }

  Mesh engineMesh;
  engineMesh.Vertices = vertices;
  engineMesh.Indices = indices;
  return engineMesh;
}

std::string FileModelBuilder::toString(fastgltf::AccessorType accessorType) {
  switch (accessorType) {
    case fastgltf::AccessorType::Invalid:
      return "Invalid";
    case fastgltf::AccessorType::Mat2:
      return "Mat2";
    case fastgltf::AccessorType::Mat3:
      return "Mat3";
    case fastgltf::AccessorType::Mat4:
      return "Mat4";
    case fastgltf::AccessorType::Scalar:
      return "Scalar";
    case fastgltf::AccessorType::Vec2:
      return "Vec2";
    case fastgltf::AccessorType::Vec3:
      return "Vec3";
    case fastgltf::AccessorType::Vec4:
      return "Vec4";
    default:
      throw std::format_error("Accessor type not recognised");
  }
}
std::string FileModelBuilder::toString(fastgltf::ComponentType componentType) {
  switch (componentType) {
    case fastgltf::ComponentType::Byte:
      return "Byte";
    case fastgltf::ComponentType::Double:
      return "Double";
    case fastgltf::ComponentType::Float:
      return "Float";
    case fastgltf::ComponentType::Int:
      return "Int";
    case fastgltf::ComponentType::Invalid:
      return "Invalid";
    case fastgltf::ComponentType::Short:
      return "Short";
    case fastgltf::ComponentType::UnsignedByte:
      return "UnsignedByte";
    case fastgltf::ComponentType::UnsignedInt:
      return "UnsignedInt";
    case fastgltf::ComponentType::UnsignedShort:
      return "UnsignedShort";
    default:
      throw std::format_error("Component type not recognised");
  }
}

std::string FileModelBuilder::toString(fastgltf::MimeType mimeType) {
  switch (mimeType) {
    case fastgltf::MimeType::DDS:
      return "DDS";
    case fastgltf::MimeType::GltfBuffer:
      return "GltfBuffer";
    case fastgltf::MimeType::JPEG:
      return "JPEG";
    case fastgltf::MimeType::KTX2:
      return "KTX2";
    case fastgltf::MimeType::None:
      return "None";
    case fastgltf::MimeType::OctetStream:
      return "OctetStream";
    case fastgltf::MimeType::PNG:
      return "PNG";
    case fastgltf::MimeType::WEBP:
      return "WEBP";
  }
}

}