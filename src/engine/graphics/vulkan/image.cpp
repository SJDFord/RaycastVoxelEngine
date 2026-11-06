#include "image.hpp"

#include "./builders/image_create_info_builder.hpp"
#include "./builders/image_view_create_info_builder.hpp"

#include <print>

namespace engine {

// TODO: See if we can get away with using a CommandBuffer here instead of the OneTimeCommandSubmitter
Image::Image(
    vk::Device const &         device,
    vk::PhysicalDevice const & physicalDevice,
    OneTimeCommandSubmitter& oneTimeCommandSubmitter,
    vk::Format format,
    void* data,
    uint32_t dataSize,
    vk::Extent2D extent,
	bool isCubeMap
  ): Image::Image(
		device,
		physicalDevice, 
		oneTimeCommandSubmitter, 
		format,
		data, 
		dataSize, 
		extent, 
		1, 
		[](uint32_t mipLevel, uint32_t layer, uint32_t faceSlice) { return 0; },
		[](uint32_t mipLevel) { return 0; },
		isCubeMap
	) {

}

Image::Image(
    vk::Device const &         device,
    vk::PhysicalDevice const & physicalDevice,
    engine::OneTimeCommandSubmitter& oneTimeCommandSubmitter,
    vk::Format format,
    void* data,
    uint32_t dataSize,
    vk::Extent2D extent,
    uint32_t mipLevels,
    std::function<uint32_t(uint32_t mipLevel, uint32_t layer, uint32_t faceSlice)> getOffset,
    std::function<uint32_t(uint32_t mipLevel)> getImageSize,
	bool isCubeMap
) : _device{device}, _physicalDevice{physicalDevice} {
    
	// TODO: Clean this up and incorporate it into engine::Image
	// ktx1 doesn't know whether the content is sRGB or linear, but most tools save in sRGB, so assume that.
	//constexpr vk::Format format = vk::Format::eR8G8B8A8Srgb; //vk::Format::eR8G8B8A8Unorm; //vk::Format::eR8G8B8A8Srgb;

	// We prefer using staging to copy the texture data to a device local optimal image
	vk::Bool32 useStaging = true;

	// Only use linear tiling if forced
	bool forceLinearTiling = false;
	if (forceLinearTiling)
	{
		// Don't use linear if format is not supported for (linear) shader sampling
		// Get device properties for the requested texture format
		vk::FormatProperties formatProperties = physicalDevice.getFormatProperties(format);
		useStaging                            = !(formatProperties.linearTilingFeatures & vk::FormatFeatureFlagBits::eSampledImage);
	}
	
	auto layerCount = isCubeMap ? 6 : 1; // One per cube face
	auto levelCount = useStaging ? mipLevels : 1;
	vk::ImageCreateInfo imageCreateInfo = vk::ImageCreateInfo()
		.setImageType(vk::ImageType::e2D)
		.setFormat(format)
		.setExtent(vk::Extent3D(extent, 1))
		.setMipLevels(levelCount)
		.setArrayLayers(layerCount)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setTiling(useStaging ? vk::ImageTiling::eOptimal : vk::ImageTiling::eLinear)
		.setUsage(useStaging ? vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled : vk::ImageUsageFlagBits::eSampled)
		.setSharingMode(vk::SharingMode::eExclusive)
		.setInitialLayout(useStaging ? vk::ImageLayout::eUndefined : vk::ImageLayout::ePreinitialized)
		.setFlags(isCubeMap ? vk::ImageCreateFlagBits::eCubeCompatible : vk::ImageCreateFlags());

	_textureImage = device.createImage(imageCreateInfo);

	vk::MemoryRequirements memoryRequirements   = device.getImageMemoryRequirements(_textureImage);
	uint32_t memoryType           = engine::findMemoryType(physicalDevice.getMemoryProperties(), memoryRequirements.memoryTypeBits, 
		useStaging ? vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent : vk::MemoryPropertyFlagBits::eDeviceLocal);
	vk::MemoryAllocateInfo memoryAllocateInfo  = vk::MemoryAllocateInfo()
		.setAllocationSize(memoryRequirements.size)
		.setMemoryTypeIndex(memoryType);
	_textureImageMemory = device.allocateMemory(memoryAllocateInfo);
	device.bindImageMemory(_textureImage, _textureImageMemory, 0);

	if (useStaging) {
		engine::Buffer stagingBuffer(
			device, 
			physicalDevice, 
			dataSize,
			1,
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
		);

		// Copy texture data into host local staging buffer
		stagingBuffer.map(dataSize, 0);
		stagingBuffer.writeToBuffer(data, dataSize);
		stagingBuffer.unmap();

		std::vector<vk::BufferImageCopy> bufferCopyRegions;

		for (uint32_t layer = 0; layer < layerCount; layer++)
		{
			for (uint32_t i = 0; i < mipLevels; i++)
			{
				uint32_t offset = getOffset(i, 0, layer);

				vk::ImageSubresourceLayers imageSubresourceLayers = vk::ImageSubresourceLayers()
					.setAspectMask(vk::ImageAspectFlagBits::eColor)
					.setMipLevel(i)
					.setBaseArrayLayer(layer)
					.setLayerCount(1);

				vk::BufferImageCopy bufferImageCopy = vk::BufferImageCopy()
					.setImageSubresource(imageSubresourceLayers)
					.setImageExtent(vk::Extent3D(extent.width >> i, extent.height >> i, 1))
					.setBufferOffset(offset);

				bufferCopyRegions.push_back(bufferImageCopy);
			}
		}	

		transitionImageLayout(oneTimeCommandSubmitter, _textureImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, 1, layerCount);

		oneTimeCommandSubmitter.submit([&] (const vk::CommandBuffer& commandBuffer) {
			// Copy mip levels from staging buffer
			stagingBuffer.copyTo(commandBuffer, _textureImage, bufferCopyRegions);
		});

		transitionImageLayout(oneTimeCommandSubmitter, _textureImage, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, 1, layerCount);
	} else {
		void *data = device.mapMemory(_textureImageMemory, 0, memoryRequirements.size);
		// Copy image data of the first mip level into memory
		//memcpy(data, data, ktxTexture_GetImageSize(ktx_texture, 0));
		memcpy(data, data, getImageSize(0));
		device.unmapMemory(_textureImageMemory);

		transitionImageLayout(oneTimeCommandSubmitter, _textureImage, vk::ImageLayout::ePreinitialized, vk::ImageLayout::eShaderReadOnlyOptimal, 1, layerCount);
	}

	// now, the ktx_texture can be destroyed
	//ktxTexture_Destroy(ktx_texture);

	// Create a texture sampler
	// In Vulkan textures are accessed by samplers
	// This separates all the sampling information from the texture data. This means you could have multiple sampler objects for the same texture with different settings
	// Note: Similar to the samplers available with OpenGL 3.3

	// Enable anisotropic filtering
	// This feature is optional, so we must check if it's supported on the device
	float maxAnisotropy = 1.0f;
	auto deviceFeatures =  physicalDevice.getFeatures();
	if (deviceFeatures.samplerAnisotropy)
	{
		// Use max. level of anisotropy for this example
		maxAnisotropy = physicalDevice.getProperties().limits.maxSamplerAnisotropy;
	}

	const vk::FormatProperties formatProperties = physicalDevice.getFormatProperties(format);

	bool hasLinearFilter = !!(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear);

	vk::SamplerCreateInfo samplerCreateInfo = vk::SamplerCreateInfo()
		.setMagFilter(hasLinearFilter ? vk::Filter::eLinear : vk::Filter::eNearest)
		.setMinFilter(hasLinearFilter ? vk::Filter::eLinear : vk::Filter::eNearest)
		.setMipmapMode(hasLinearFilter ? vk::SamplerMipmapMode::eLinear : vk::SamplerMipmapMode::eNearest)
		.setAddressModeU(vk::SamplerAddressMode::eClampToEdge)
		.setAddressModeV(vk::SamplerAddressMode::eClampToEdge)
		.setAddressModeW(vk::SamplerAddressMode::eClampToEdge)
		.setMaxAnisotropy(1.0f < maxAnisotropy)
		.setCompareOp(vk::CompareOp::eNever)
		.setMinLod(0.0f)
		.setMaxLod((useStaging) ? static_cast<float>(mipLevels) : 0.0f)
		.setBorderColor(vk::BorderColor::eFloatOpaqueWhite);

	_textureSampler = _device.createSampler(samplerCreateInfo);
	// Create image view
	// Textures are not directly accessed by the shaders and
	// are abstracted by image views containing additional
	// information and sub resource ranges
	vk::ImageSubresourceRange subresourceRange = vk::ImageSubresourceRange()
		.setAspectMask(vk::ImageAspectFlagBits::eColor)
		.setBaseMipLevel(0)
		.setBaseArrayLayer(0)
		.setLayerCount(layerCount)
		.setLevelCount(levelCount);

	vk::ImageViewCreateInfo imageViewCreateInfo = vk::ImageViewCreateInfo()
		.setViewType(isCubeMap ? vk::ImageViewType::eCube : vk::ImageViewType::e2D)
		.setFormat(format)
		.setComponents({vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA})
		.setSubresourceRange(subresourceRange)
		.setImage(_textureImage);

	_textureImageView          = device.createImageView(imageViewCreateInfo);
}

Image::~Image() {
    _device.destroySampler(_textureSampler);
    _device.destroyImageView(_textureImageView);
    _device.destroyImage(_textureImage);
    _device.freeMemory(_textureImageMemory);
}

vk::DescriptorImageInfo Image::descriptorInfo() {
  vk::DescriptorImageInfo imageInfo = vk::DescriptorImageInfo()
      .setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
      .setImageView(_textureImageView)
      .setSampler(_textureSampler);
  return imageInfo;
}

void Image::transitionImageLayout(
	engine::OneTimeCommandSubmitter& oneTimeCommandSubmitter,
    vk::Image image, 
	vk::ImageLayout oldLayout, 
	vk::ImageLayout newLayout, 
	uint32_t levelCount,
	uint32_t layerCount
  ) {
  vk::ImageSubresourceRange subresource = vk::ImageSubresourceRange()
      .setAspectMask(vk::ImageAspectFlagBits::eColor)
      .setBaseMipLevel(0)
      .setLevelCount(levelCount)
      .setLayerCount(layerCount);
  
  vk::ImageMemoryBarrier barrier = vk::ImageMemoryBarrier()
    .setOldLayout(oldLayout)
    .setNewLayout(newLayout)
    .setSrcQueueFamilyIndex(vk::QueueFamilyIgnored)
    .setDstQueueFamilyIndex(vk::QueueFamilyIgnored)
    .setImage(image)
    .setSubresourceRange(subresource);

  vk::PipelineStageFlags sourceStage;
  vk::PipelineStageFlags destinationStage;

  if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
    barrier.srcAccessMask = vk::AccessFlagBits::eNone;
    barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
    sourceStage = vk::PipelineStageFlagBits::eHost;
    destinationStage = vk::PipelineStageFlagBits::eTransfer;
  } else if (
      oldLayout == vk::ImageLayout::eTransferDstOptimal &&
      newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
    barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

    sourceStage = vk::PipelineStageFlagBits::eTransfer;
    destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
  } else if (
      oldLayout == vk::ImageLayout::ePreinitialized &&
      newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
    barrier.srcAccessMask = vk::AccessFlagBits::eHostWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

    sourceStage = vk::PipelineStageFlagBits::eHost;
    destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
  } else {
    throw std::invalid_argument("unsupported layout transition!");
  }

  oneTimeCommandSubmitter.submit([sourceStage, destinationStage, barrier](const vk::CommandBuffer& commandBuffer) {
    commandBuffer.pipelineBarrier(sourceStage, destinationStage, vk::DependencyFlags(), {}, {}, barrier);
  });
}

}