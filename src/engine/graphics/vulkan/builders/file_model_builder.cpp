#include "file_model_builder.hpp"

namespace engine {

FileModelBuilder::FileModelBuilder(
    vk::Device const &         device,
    vk::PhysicalDevice const & physicalDevice
  ) : _device{device}, _physicalDevice{physicalDevice} {

}

FileModelBuilder::~FileModelBuilder() {
    
}


std::unique_ptr<engine::Model> FileModelBuilder::build(OneTimeCommandSubmitter& oneTimeCommandSubmitter, std::string filePath, std::vector<std::shared_ptr<Image>>& images) {
    if (!filePath.ends_with(".gltf")) {
        throw std::format_error("File types other than GLTF are not supported");
    }
    //std::filesystem::path p = filePath;
    //std::string directory = p.has_parent_path() ? p.parent_path().string() : "";
    fastgltf::Parser parser;

    auto data = fastgltf::GltfDataBuffer::FromPath(filePath);
    if (data.error() != fastgltf::Error::None) {
      throw std::format_error("Could not parse gltf file");
    }

    // This loads the glTF file into the gltf object and parses the JSON.
    // It automatically detects whether this is a JSON-based or binary glTF.
    // If you know the type, you can also use loadGltfJson or loadGltfBinary.
    // TODO: Use file system for this
    auto lastSlashIndex = filePath.find_last_of("/");
    _directory = filePath.substr(0, lastSlashIndex);
    _modelName = filePath.substr(lastSlashIndex + 1);
    constexpr auto gltfOptions = 
      fastgltf::Options::DontRequireValidAssetMember | 
      //fastgltf::Options::AllowDouble | 
      fastgltf::Options::LoadGLBBuffers | 
      // WARN: Don't use this option as fastgltf mistakes KTX images for JPEG or PNG 
      fastgltf::Options::LoadExternalImages |
      fastgltf::Options::LoadExternalBuffers;
  
    auto assetResult = parser.loadGltf(data.get(), _directory, gltfOptions);
    if (auto error = assetResult.error(); error != fastgltf::Error::None) {
      throw std::format_error("Could not load gltf file");
    }

    fastgltf::Asset& asset = assetResult.get();
    // TODO: Mesh to image index mapping needed
    std::map<int, int> materialToImageMap = {};
    for (uint32_t i = 0; i < asset.materials.size(); i++) {
      auto& material = asset.materials[i];
      if (material.pbrData.baseColorTexture.has_value()) {
        auto& textureInfo = material.pbrData.baseColorTexture.value();
        auto textureIndex = textureInfo.textureIndex;
        auto &texture = asset.textures[textureIndex];
        if (texture.imageIndex.has_value()) {
          materialToImageMap[i] = texture.imageIndex.value();
          std::println("Model {}: Mapped material index {} to model image index {}", _modelName, i, texture.imageIndex.value());
        }

        //std::println("Model {}: Material: {} Texture Index: {}", _modelName, material.name, textureIndex);
      }
    }

    std::map<int, int> imageToEngineImageMap;
    for (uint32_t i = 0; i < asset.images.size(); i++) {
      auto& image = asset.images[i];
      //std::println("Processing image {}", i);
      /*
      if (filePath.contains("Sponza") && i > 10) {
        continue;
      }
      */

      //int engineImageIndex = 19;
      auto engineImageIndex = createImage(asset, image, oneTimeCommandSubmitter, images);
      if (engineImageIndex < 0) {
        continue;
      }
      imageToEngineImageMap[i] = engineImageIndex;
      std::println("Model {}: Created engine image at index {} for model image index {}", _modelName, engineImageIndex, i);
    }

    auto initialSceneIndex = asset.defaultScene.value_or(0);
    auto& scene = asset.scenes[initialSceneIndex];

    auto initial = fastgltf::math::fmat4x4();

    std::vector<std::vector<std::pair<Mesh, int>>> meshes;
    //std::println("Model {}: {} meshes", _modelName, asset.meshes.size());
    meshes.resize(asset.meshes.size());

    auto processNode = [&](std::size_t nodeIndex, fastgltf::math::fmat4x4 nodeMatrix, auto& self) -> void {
      std::println("Model {}: {} nodes", _modelName, asset.nodes.size());
      assert(asset.nodes.size() > nodeIndex);
      auto& node = asset.nodes[nodeIndex];
      nodeMatrix = fastgltf::getTransformMatrix(node, nodeMatrix);

      if (node.meshIndex.has_value()) {
          std::size_t meshIndex = node.meshIndex.value();
          fastgltf::Mesh& mesh = asset.meshes[meshIndex];
          std::vector<std::pair<Mesh, int>> engineMeshes = createMeshes(asset, mesh, oneTimeCommandSubmitter);
          meshes.push_back(engineMeshes);
      }

      for (auto& child : node.children) {
        self(child, nodeMatrix, self);
      }
    };

    for (auto& sceneNode : scene.nodeIndices) {
      processNode(sceneNode, initial, processNode);
    }

    std::vector<std::pair<Mesh, int>> flattenedMeshes;

    for (auto& meshList : meshes) {
      for (auto &meshPair : meshList) {
        if (meshPair.first.Vertices.size() < 1) {
          continue;
        }

        auto modelMaterialIndex = meshPair.second;
        auto modelImageIndex = materialToImageMap[modelMaterialIndex];
        auto engineImageIndex = imageToEngineImageMap[modelImageIndex];
        
        //std::println("Model {}: Mapped model material index {} to engine image index {}", _modelName, modelMaterialIndex, engineImageIndex);
        flattenedMeshes.push_back({meshPair.first, engineImageIndex});
      }
    }

    return std::make_unique<engine::Model>(_device, _physicalDevice, oneTimeCommandSubmitter, flattenedMeshes);
}

std::vector<std::pair<Mesh, int>> FileModelBuilder::createMeshes(const fastgltf::Asset& asset, const fastgltf::Mesh& mesh, OneTimeCommandSubmitter& oneTimeCommandSubmitter) {
  auto& primitives = mesh.primitives;
  std::println("Model {}: Mesh {}: {} primitives, {} textures, {} images, {} materials", _modelName, mesh.name, primitives.size(), asset.textures.size(), asset.images.size(), asset.materials.size());

  std::vector<std::pair<Mesh, int>> meshes;
  meshes.resize(primitives.size());
  
  for (const fastgltf::Primitive& primitive : primitives) {
    meshes.push_back(createMesh(asset, mesh, primitive, oneTimeCommandSubmitter));
  }

  return meshes;
}


std::pair<Mesh, int> FileModelBuilder::createMesh(const fastgltf::Asset& asset, const fastgltf::Mesh& mesh, const fastgltf::Primitive& primitive, OneTimeCommandSubmitter& oneTimeCommandSubmitter) {
  std::println("Model {}: Mesh {}: Primitive: {}", _modelName, mesh.name, "");
  std::vector<Vertex> vertices;
  if (primitive.type != fastgltf::PrimitiveType::Triangles) {
    throw std::format_error("Encountered unsupported primitive type");
  }

  const fastgltf::Attribute* positionAttribute = primitive.findAttribute("POSITION");
  if (!positionAttribute) {
    throw std::format_error("Missing position attribute");
  }

  //primitive.
  /*
  for (auto& material : asset.materials) {
    if (material.pbrData.baseColorTexture.has_value()) {
      auto& textureInfo = material.pbrData.baseColorTexture.value();
      auto textureIndex = textureInfo.textureIndex;
      
      std::println("Model {}: Material: {} Texture Index: {}", _modelName, material.name, textureIndex);
    }
  }

  for (auto& image : asset.images) {
    images.emplace_back(createImage(asset, image, oneTimeCommandSubmitter));
  }
  */
  auto& positionAccessor = asset.accessors[positionAttribute->accessorIndex];
  //std::println("Model {}: Mesh {}: Primitive: {}: Vertices: {}", _modelName, mesh.name, "", positionAccessor.count);
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
  //std::println("Model {}: Mesh {}: Primitive: {}: Normals: {}", _modelName, mesh.name, "", normalAccessor.count);
  fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(asset, normalAccessor, [&](fastgltf::math::fvec3 pos, std::size_t idx) {
    //std::println("Adding normal {}", idx);
    vertices[idx].normal = glm::vec3(pos.x(), pos.y(), pos.z());
  });

  const fastgltf::Attribute* textCoordAttribute = primitive.findAttribute("TEXCOORD_0");

  if (!textCoordAttribute) {
    throw std::format_error("Missing normal attribute");
  }

  auto& texCoordAccessor = asset.accessors[textCoordAttribute->accessorIndex];
  //std::println("Model {}: Mesh {}: Primitive: {}: Tex Coords: {} {} {}", _modelName, mesh.name, "", texCoordAccessor.count, toString(texCoordAccessor.type), toString(texCoordAccessor.componentType));
  
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

  //std::println("Reading indices");

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
  int materialIndex = -1;
  if (primitive.materialIndex.has_value()) {
    materialIndex = primitive.materialIndex.value();
  }
  return {engineMesh, materialIndex};
}


int FileModelBuilder::createImage(const fastgltf::Asset& asset, const fastgltf::Image& image, OneTimeCommandSubmitter& oneTimeCommandSubmitter, std::vector<std::shared_ptr<Image>>& images) {
    int width, height, nrChannels;
   
    int engineImageIndex = -1;

    std::println("Model {}: Image: {}", _modelName, image.name);

    /*
    try
    {

      auto &bufView = std::get<fastgltf::sources::BufferView>(image.data);
      std::println("Model {}: Image: {}: BufferViewIndex: {}", _modelName, image.name, bufView.bufferViewIndex);
    }
    catch (std::bad_variant_access const& ex)
    {
      std::println("Model {}: Image: {}: Data is not BufferView", _modelName, image.name);
    }
    */
    // std::monostate, sources::BufferView, sources::URI, sources::Array, sources::Vector, sources::CustomBuffer, sources::ByteView, sources::Fallback
    std::visit(
        fastgltf::visitor {
            [](auto& arg) {
            },
            [&](fastgltf::sources::Array array) {
              //std::println("Model {}: Image: {}: Array: {} ({})", _modelName, image.name, array.bytes.size(), toString(array.mimeType));
              engineImageIndex = readPngStb(array.bytes.data(), array.bytes.size(), oneTimeCommandSubmitter, images);                 
            },
           [&](fastgltf::sources::CustomBuffer customBuffer) {
              //std::println("Model {}: Image: {}: Custom Buffer: {} ({})", _modelName, image.name, customBuffer.id, toString(customBuffer.mimeType));
                                    
            },
            [&](fastgltf::sources::ByteView byteView) {
              //std::println("Model {}: Image: {}: Byte View: {} ({})", _modelName, image.name, byteView.bytes.size(), toString(byteView.mimeType));
                                    
            },
            [&](fastgltf::sources::URI filePath) {
                //std::println("Model {}: Image: {}: URI: {}", _modelName, image.name, filePath.uri.c_str());
                //FileImageBuilder fileImageBuilder(_device, _physicalDevice);

    
    
    auto assetPath = _directory + "/" + filePath.uri.c_str();
    int texWidth, texHeight, texChannels;
    stbi_uc *pixels = stbi_load(assetPath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    vk::DeviceSize imageSize = texWidth * texHeight * 4;
    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    std::shared_ptr<engine::Image> engineImage = std::make_shared<engine::Image>(
        _device, 
        _physicalDevice, 
        oneTimeCommandSubmitter,
        vk::Format::eR8G8B8A8Srgb,
        pixels,
        imageSize,
        vk::Extent2D(texWidth, texHeight),
        false
    );

    stbi_image_free(pixels);
                //std::shared_ptr<Image> engineImage  = fileImageBuilder.build(oneTimeCommandSubmitter, _directory + "/" + filePath.uri.c_str());
                //std::shared_ptr<Image> engineImage = uniqueIMage;
                //std::unique_ptr<Image> engineImage = fileImageBuilder.build(oneTimeCommandSubmitter, _directory + "/" + filePath.uri.c_str())
                //std::shared_ptr<Image> engineImage = fileImageBuilder.build(oneTimeCommandSubmitter, _directory + "/" + filePath.uri.c_str());
                images.push_back(engineImage);
                engineImageIndex = images.size() - 1;
                /*
                assert(filePath.fileByteOffset == 0); // We don't support offsets with stbi.
                assert(filePath.uri.isLocalPath()); // We're only capable of loading
                                                    // local files.

                const std::string path(filePath.uri.path().begin(),
                    filePath.uri.path().end()); // Thanks C++.
                unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 4);
                if (data) {
                    VkExtent3D imagesize;
                    imagesize.width = width;
                    imagesize.height = height;
                    imagesize.depth = 1;

                    //newImage = engine->create_image(data, imagesize, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT,false);

                    stbi_image_free(data);
                }
                */
            },
            [&](fastgltf::sources::Vector vector) {


                //std::println("Model {}: Image: {}: Vector: {}", _modelName, image.name, vector.bytes.size());
                /*
                unsigned char* data = stbi_load_from_memory(vector.bytes.data(), static_cast<int>(vector.bytes.size()),
                    &width, &height, &nrChannels, 4);
                if (data) {
                    VkExtent3D imagesize;
                    imagesize.width = width;
                    imagesize.height = height;
                    imagesize.depth = 1;

                    //newImage = engine->create_image(data, imagesize, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT,false);

                    stbi_image_free(data);
                }
                */
            },
            [&](fastgltf::sources::BufferView view) {
                //std::println("Model {}: Image: {}: BufferViewIndex: {}", _modelName, image.name, view.bufferViewIndex);
                auto& bufferView = asset.bufferViews[view.bufferViewIndex];
                auto& buffer = asset.buffers[bufferView.bufferIndex];
                auto& dataSource = buffer.data;
                    
                    try
                    {

                      auto &array = std::get<fastgltf::sources::Array>(dataSource);
                      //std::println("Model {}: Image: {}: BufferViewIndex: {}: Bytes: {}", _modelName, image.name, view.bufferViewIndex, array.bytes.size());
                    }
                    catch (std::bad_variant_access const& ex)
                    {
                      //std::println("Model {}: Image: {}: Data is not Array", _modelName, image.name);
                    }
                // std::monostate, sources::BufferView, sources::URI, sources::Array, sources::Vector, sources::CustomBuffer, sources::ByteView, sources::Fallback
  
                // TODO: Make sure LoadExternalBuffers is specified
                std::visit(fastgltf::visitor { // We only care about VectorWithMime here, because we
                                               // specify LoadExternalBuffers, meaning all buffers
                                               // are already loaded into a vector.
                               [](auto& arg) {},

                               [&](fastgltf::sources::URI uri) {
                                 //std::println("Model {}: Image: {}: BufferViewIndex: {}: URI: {} ({})", _modelName, image.name, view.bufferViewIndex, uri.uri.c_str(), toString(uri.mimeType));
                                  
                               },
                               [&](fastgltf::sources::Vector vector) {
                                  //std::println("Model {}: Image: {}: BufferViewIndex: {}: Vector Bytes: {} ({})", _modelName, image.name, view.bufferViewIndex, vector.bytes.size(), toString(vector.mimeType));
                                  engineImageIndex = readPngStb(vector.bytes.data() + bufferView.byteOffset, bufferView.byteLength, oneTimeCommandSubmitter, images);
                               },
                               [&](fastgltf::sources::Array array) { 
                                  //std::println("Model {}: Image: {}: BufferViewIndex: {}: Array Bytes: {} ({}) Buffer View: {} (offset) {} (length)", _modelName, image.name, view.bufferViewIndex, array.bytes.size(), toString(array.mimeType), bufferView.byteOffset, bufferView.byteLength);
                                  engineImageIndex = readPngStb(array.bytes.data() + bufferView.byteOffset, bufferView.byteLength, oneTimeCommandSubmitter, images);
                               } },
                    dataSource);
            },
        },
        image.data);
    
   return engineImageIndex;
}

int FileModelBuilder::createImage(
    fastgltf::sources::Array& array, 
    OneTimeCommandSubmitter& oneTimeCommandSubmitter, 
    std::vector<std::shared_ptr<Image>>& images) {
             
  bool success = false;
  //auto engineImage = readPngStb(array.bytes.data(), array.bytes.size(), oneTimeCommandSubmitter, &success);
  return -1;
  /*
  // auto engineImage = readPngLibpng(array.bytes.data() + bufferView.byteOffset, bufferView.byteLength, oneTimeCommandSubmitter, &success);
  if (success) {
    images.push_back(engineImage);
    engineImageIndex = images.size() - 1;
  } 

  bool success = false;
  auto engineImage = readPngStb(array.bytes.data() + bufferView.byteOffset, bufferView.byteLength, oneTimeCommandSubmitter, &success);

  // auto engineImage = readPngLibpng(array.bytes.data() + bufferView.byteOffset, bufferView.byteLength, oneTimeCommandSubmitter, &success);
  if (success) {
    images.push_back(engineImage);
    engineImageIndex = images.size() - 1;
  }
  */
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
    default:
      throw std::invalid_argument("mimeType");
  }
}


int FileModelBuilder::readPngStb(std::byte *data, std::size_t size, OneTimeCommandSubmitter& oneTimeCommandSubmitter, std::vector<std::shared_ptr<Image>>& images) {  
  int width, height, nrChannels; 
  int engineImageIndex = -1;
  //stbi_uc *pixelData = stbi_load(tempFileName.c_str(), &width, &height, &nrChannels, STBI_rgb_alpha);
  
  unsigned char* pixelData = stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(data),
    static_cast<int>(size),
    &width, &height, &nrChannels, STBI_rgb_alpha);

  //if (nrChannels == 4) {
    for (int i = 0; i < width * height; i++) {
      //pixelData[i*4] = 255;
      //pixelData[(i*4) + 3] = 255;
      /*
      pixelData[i + 1] = 128;
      pixelData[i + 2] = 128;
      pixelData[i + 3] = 128;
      */
    }
  //}
  auto sampleR = static_cast<int>(pixelData[0]);
  auto sampleG = static_cast<int>(pixelData[1]);
  auto sampleB = static_cast<int>(pixelData[2]);
  auto sampleA = static_cast<int>(pixelData[3]);

  //std::println("readpngstb {}x{} ({}) Sample: {} {} {} {}", width, height, nrChannels, sampleR, sampleG, sampleB, sampleA);
  
  if (pixelData) {
    auto engineImage = std::make_shared<engine::Image>(
      _device, 
      _physicalDevice, 
      oneTimeCommandSubmitter,
      vk::Format::eR8G8B8A8Srgb,
      pixelData,
      width * height * STBI_rgb_alpha,
      vk::Extent2D(width, height),
      false
    );
    images.push_back(engineImage);
    engineImageIndex = images.size() - 1;
  }

  //newImage = engine->create_image(data, imagesize, VK_FORMAT_R8G8B8A8_UNORM,
  //    VK_IMAGE_USAGE_SAMPLED_BIT,false);

  stbi_image_free(pixelData);

  return engineImageIndex;
}

static int
png_rgba_pixel_limit(png_uint_32 w, png_uint_32 h)
{
double da;
/* assert(w != 0 && h != 0); */
if (w > PNG_RGBA_PIXEL_LIMIT || h > PNG_RGBA_PIXEL_LIMIT)
return (1); /* since both (w) and (h) are non-zero. */
/* since an IEEE-754 double has a 53 bit mantissa, it can
* represent the maximum area: (w * h == 2^48) exactly. */
da = ((double)w) * ((double)h);
if (da > ((double)PNG_RGBA_PIXEL_LIMIT))
return (1);
return (0); /* the PNG image is within the pixel limit. */
}

void ReadDataFromInputStream(
png_structp png_ptr, png_byte* raw_data, png_size_t read_length) {
ReadDataHandle* handle = (ReadDataHandle*)png_get_io_ptr(png_ptr);
const png_byte* png_src = handle->data.data + handle->offset;
memcpy(raw_data, png_src, read_length);
handle->offset += read_length;
}

int FileModelBuilder::readPngLibpng(std::byte *data, std::size_t size, OneTimeCommandSubmitter& oneTimeCommandSubmitter, std::vector<std::shared_ptr<Image>>& images) {
  
  std::println("Reading PNG using libpng");
  int width, height, nrChannels; 
  int engineImageIndex = -1;
  png_byte magic[PNG_SIG_BYTES]; /* (signature byte buffer) */
png_structp png_ctx;
png_infop info_ctx;
png_uint_32 img_width, img_height, row;
png_byte img_depth, img_color_type;
/* 'volatile' qualifier forces reload in setjmp cleanup: */
png_byte *volatile img_data = NULL;
png_bytep *volatile row_data = NULL;
;//*buf = NULL;
/* it is assumed that 'longjmp' can be invoked within this
* code to efficiently unwind resources for *all* errors. */
/* PNG structures and resource unwinding: */
if ((png_ctx = png_create_read_struct(
PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)) == NULL) {
  // TODO: Throw
  return -1;
  //return (1); /* ENOMEM (?) */
}

if ((info_ctx = png_create_info_struct(png_ctx)) == NULL)
{
png_destroy_read_struct(&png_ctx, NULL, NULL);
//return (1); /* ENOMEM (?) */
  return -1;
}
if (setjmp(png_jmpbuf(png_ctx)) != 0)
{
png_destroy_read_struct(&png_ctx, &info_ctx, NULL);
free(img_data); free(row_data);
//return (1); /* libpng feedback (?) */
  return -1;
}

/* check PNG file signature: */
//if (fread(magic, (1), PNG_SIG_BYTES, fp) != PNG_SIG_BYTES)
// png_error(png_ctx, "invalid PNG file");
//Read(magic, PNG_SIG_BYTES);
std::copy(reinterpret_cast<unsigned char*>(data + 0), reinterpret_cast<unsigned char*>(data + PNG_SIG_BYTES), magic);
//return byteCount;

if (png_sig_cmp(magic, 0, PNG_SIG_BYTES))
png_error(png_ctx, "invalid PNG file");
/* set the input file stream and get the PNG image info: */
//png_init_io(png_ctx, fp);
ReadDataHandle a = ReadDataHandle{ { reinterpret_cast<png_byte*>(data), 898 }, 0 };
png_set_read_fn(png_ctx, &a, ReadDataFromInputStream);
//png_set_read_fn(png_ptr, NULL, ReadDataFromInputStream);
//png_set_sig_bytes(png_ctx, PNG_SIG_BYTES);
//////////////// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
png_read_info(png_ctx, info_ctx);
img_width = png_get_image_width(png_ctx, info_ctx);
img_height = png_get_image_height(png_ctx, info_ctx);
#if (1) /* PNG doesn't support zero area image? */
if (img_width == 0 || img_height == 0)
png_error(png_ctx, "zero area PNG image");
#endif
if (png_rgba_pixel_limit(img_width, img_height))
png_error(png_ctx, "PNG image exceeds pixel limits");
img_depth = png_get_bit_depth(png_ctx, info_ctx);
img_color_type = png_get_color_type(png_ctx, info_ctx);
/* force 8-bit color channels: */
if (img_depth == 16)
png_set_strip_16(png_ctx);
else if (img_depth < 8)
png_set_packing(png_ctx);
/* force formats to RGB: */
if (img_color_type != PNG_COLOR_TYPE_RGBA)
png_set_expand(png_ctx);
if (img_color_type == PNG_COLOR_TYPE_PALETTE)
png_set_palette_to_rgb(png_ctx);
if (img_color_type == PNG_COLOR_TYPE_GRAY)
png_set_gray_to_rgb(png_ctx);
if (img_color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
png_set_gray_to_rgb(png_ctx);
/* add full opacity alpha channel if required: */
if (img_color_type != PNG_COLOR_TYPE_RGBA)
png_set_filler(png_ctx, 0xff, PNG_FILLER_AFTER);
/* apply the output transforms before reading image data: */
png_read_update_info(png_ctx, info_ctx);
/* allocate RGBA image data: */
img_data = (png_byte *)
malloc((size_t)(img_width * img_height * (4)));
if (img_data == NULL)
png_error(png_ctx, "error allocating image buffer");
/* allocate row pointers: */
row_data = (png_bytep *)
malloc((size_t)(img_height * sizeof(png_bytep)));
if (row_data == NULL)
png_error(png_ctx, "error allocating row pointers");
/* set the row pointers and read the RGBA image data: */
for (row = 0; row < img_height; row++)
row_data[row] = img_data +
(img_height - (row + 1)) * (img_width * (4));
png_read_image(png_ctx, row_data);
/* libpng and dynamic resource unwinding: */
png_read_end(png_ctx, NULL);

png_destroy_read_struct(&png_ctx, &info_ctx, NULL);
free(row_data);
//*w = (unsigned) img_width, *h = (unsigned) img_height;
width = img_width;
height = img_height;
//*buf = img_data; /* (asserts png_byte is an unsigned char) */

  //if (pixelData) {
    
  VkExtent3D imagesize;
    imagesize.width = width;
    imagesize.height = height;
    imagesize.depth = 1;

    auto engineImage = std::make_shared<engine::Image>(
      _device, 
      _physicalDevice, 
      oneTimeCommandSubmitter,
      vk::Format::eR8G8B8A8Srgb,
      reinterpret_cast<unsigned char*>(img_data),
      img_width * img_height * (4),
      vk::Extent2D(width, height),
      false
    );
    /*
    *success = true;
    */
    images.push_back(engineImage);
    engineImageIndex = images.size() - 1;

  //}
  return engineImageIndex;

}

}