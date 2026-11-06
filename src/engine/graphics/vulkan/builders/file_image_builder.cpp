#include "file_image_builder.hpp"
#include <print>

namespace engine {

FileImageBuilder::FileImageBuilder(
    vk::Device const &         device,
    vk::PhysicalDevice const & physicalDevice
  ) : _device{device}, _physicalDevice{physicalDevice} {

}

FileImageBuilder::~FileImageBuilder() {
    
}

std::unique_ptr<engine::Image> FileImageBuilder::build(OneTimeCommandSubmitter& oneTimeCommandSubmitter, std::string filePath, bool isCubeMap) {
  if (filePath.ends_with(".ktx")) {
    ktxTexture    *ktx_texture;
	KTX_error_code result = ktxTexture_CreateFromNamedFile(filePath.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktx_texture);
	if ((result != KTX_SUCCESS) || (ktx_texture == nullptr))
	{
		throw std::runtime_error("Couldn't load texture");
	}

    auto size = ktxTexture_GetDataSize(ktx_texture);
    vk::Format format = static_cast<vk::Format>(ktxTexture_GetVkFormat(ktx_texture));
    format = vk::Format::eR8G8B8A8Srgb; // We're forcing this format here with vk::Format::eR8G8B8A8Unorm gamma correction isn't applied automatically so colours appear washed out

    std::unique_ptr<engine::Image> image = std::make_unique<engine::Image>(
        _device, 
        _physicalDevice, 
        oneTimeCommandSubmitter,
        format,
        ktx_texture->pData,
        ktx_texture->dataSize,
        vk::Extent2D(ktx_texture->baseWidth, ktx_texture->baseHeight),
        ktx_texture->numLevels,
        [&](uint32_t mipLevel, uint32_t layer, uint32_t faceSlice) { 
            ktx_size_t offset;
            KTX_error_code result = ktxTexture_GetImageOffset(ktx_texture, mipLevel, layer, faceSlice, &offset);
            return offset;
        },
        [&](uint32_t mipLevel) { 
            return ktxTexture_GetImageSize(ktx_texture, mipLevel);
        },
        isCubeMap
    );

	ktxTexture_Destroy(ktx_texture);

    return image;
  } else {

    int texWidth, texHeight, texChannels;
    stbi_uc *pixels = stbi_load(filePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    vk::DeviceSize imageSize = texWidth * texHeight * 4;
    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    std::unique_ptr<engine::Image> image = std::make_unique<engine::Image>(
        _device, 
        _physicalDevice, 
        oneTimeCommandSubmitter,
        vk::Format::eR8G8B8A8Srgb,
        pixels,
        imageSize,
        vk::Extent2D(texWidth, texHeight),
        isCubeMap
    );

    stbi_image_free(pixels);

    return image;
  }
}

}