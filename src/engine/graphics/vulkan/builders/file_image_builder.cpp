#include "file_image_builder.hpp"

namespace engine {

FileImageBuilder::FileImageBuilder(
    vk::Device const &         device,
    vk::PhysicalDevice const & physicalDevice
  ) : _device{device}, _physicalDevice{physicalDevice} {

}

FileImageBuilder::~FileImageBuilder() {
    
}

std::unique_ptr<engine::Image> FileImageBuilder::build(OneTimeCommandSubmitter& oneTimeCommandSubmitter, std::string filePath) {
  if (filePath.ends_with(".ktx")) {
    ktxTexture    *ktx_texture;
	KTX_error_code result = ktxTexture_CreateFromNamedFile(filePath.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktx_texture);
	if ((result != KTX_SUCCESS) || (ktx_texture == nullptr))
	{
		throw std::runtime_error("Couldn't load texture");
	}

    std::unique_ptr<engine::Image> image = std::make_unique<engine::Image>(
        _device, 
        _physicalDevice, 
        oneTimeCommandSubmitter,
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
        }
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
        pixels,
        imageSize,
        vk::Extent2D(texWidth, texHeight)
    );

    stbi_image_free(pixels);

    return image;
  }
}

}