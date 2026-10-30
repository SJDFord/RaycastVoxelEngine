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
    auto assetResult = parser.loadGltf(data.get(), directory, fastgltf::Options::None);
    if (auto error = assetResult.error(); error != fastgltf::Error::None) {
      throw std::format_error("Could not load gltf file");
    }

    fastgltf::Asset& asset = assetResult.get();
    auto initialSceneIndex = asset.defaultScene.value_or(0);
    auto& scene = asset.scenes[initialSceneIndex];

    auto initial = fastgltf::math::fmat4x4();

    std::vector<std::vector<Mesh>> meshes;
    meshes.resize(asset.meshes.size());

    auto processNode = [&](std::size_t nodeIndex, fastgltf::math::fmat4x4 nodeMatrix, auto& self) -> void {
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
  
  std::vector<Mesh> meshes;
  meshes.resize(primitives.size());
  
  for (const fastgltf::Primitive& primitive : primitives) {
    meshes.push_back(createMesh(asset, primitive));
  }

  return meshes;
}


Mesh FileModelBuilder::createMesh(const fastgltf::Asset& asset, const fastgltf::Primitive& primitive) {
  std::println("Creating mesh");

  std::vector<Vertex> vertices;
  if (primitive.type != fastgltf::PrimitiveType::Triangles) {
    throw std::format_error("Encountered unsupported primitive type");
  }

  const fastgltf::Attribute* positionAttribute = primitive.findAttribute("POSITION");
  if (!positionAttribute) {
    throw std::format_error("Missing position attribute");
  }

  auto& positionAccessor = asset.accessors[positionAttribute->accessorIndex];
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
  fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(asset, normalAccessor, [&](fastgltf::math::fvec3 pos, std::size_t idx) {
    //std::println("Adding normal {}", idx);
    vertices[idx].normal = glm::vec3(pos.x(), pos.y(), pos.z());
  });

  const fastgltf::Attribute* textCoordAttribute = primitive.findAttribute("TEXCOORD_0");

  if (!textCoordAttribute) {
    throw std::format_error("Missing nofrmal attribute");
  }

  auto& texCoordAccessor = asset.accessors[textCoordAttribute->accessorIndex];
  fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec2>(asset, texCoordAccessor, [&](fastgltf::math::fvec2 uv, std::size_t idx) {
    
      //std::println("Adding uv {}", idx);
      vertices[idx].uv = glm::vec2(uv.x(), uv.y());
  });

  std::vector<std::uint32_t> indices;
  if (primitive.indicesAccessor.has_value()) {
    auto& accessor = asset.accessors[primitive.indicesAccessor.value()];
    indices.resize(accessor.count);

    fastgltf::iterateAccessorWithIndex<std::uint32_t>(asset, accessor, [&](std::uint32_t index, std::size_t idx) {
        indices[idx] = index;
    });
  }

  Mesh engineMesh;
  engineMesh.Vertices = vertices;
  engineMesh.Indices = indices;
  return engineMesh;
}

}