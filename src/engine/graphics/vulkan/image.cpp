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
    void* data,
    uint32_t dataSize,
    vk::Extent2D extent
  ): Image::Image(
		device,
		physicalDevice, 
		oneTimeCommandSubmitter, 
		data, 
		dataSize, 
		extent, 
		1, 
		[](uint32_t mipLevel, uint32_t layer, uint32_t faceSlice) { return 0; },
		[](uint32_t mipLevel) { return 0; }
	) {

}

Image::Image(
    vk::Device const &         device,
    vk::PhysicalDevice const & physicalDevice,
    engine::OneTimeCommandSubmitter& oneTimeCommandSubmitter,
    void* data,
    uint32_t dataSize,
    vk::Extent2D extent,
    uint32_t mipLevels,
    std::function<uint32_t(uint32_t mipLevel, uint32_t layer, uint32_t faceSlice)> getOffset,
    std::function<uint32_t(uint32_t mipLevel)> getImageSize
) : _device{device}, _physicalDevice{physicalDevice} {
    
	// TODO: Clean this up and incorporate it into engine::Image
	// ktx1 doesn't know whether the content is sRGB or linear, but most tools save in sRGB, so assume that.
	constexpr vk::Format format = vk::Format::eR8G8B8A8Srgb;

	auto mip_levels = mipLevels;

	// We prefer using staging to copy the texture data to a device local optimal image
	vk::Bool32 use_staging = true;

	// Only use linear tiling if forced
	bool force_linear_tiling = false;
	if (force_linear_tiling)
	{
		// Don't use linear if format is not supported for (linear) shader sampling
		// Get device properties for the requested texture format
		vk::FormatProperties format_properties = physicalDevice.getFormatProperties(format);
		use_staging                            = !(format_properties.linearTilingFeatures & vk::FormatFeatureFlagBits::eSampledImage);
	}

	if (use_staging) {

		std::println("Staging");

	}
	vk::ImageCreateInfo image_create_info = vk::ImageCreateInfo()
		.setImageType(vk::ImageType::e2D)
		.setFormat(format)
		.setExtent(vk::Extent3D(extent, 1))
		.setMipLevels(use_staging ? mip_levels : 1)
		.setArrayLayers(1)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setTiling(use_staging ? vk::ImageTiling::eOptimal : vk::ImageTiling::eLinear)
		.setUsage(use_staging ? vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled : vk::ImageUsageFlagBits::eSampled)
		.setSharingMode(vk::SharingMode::eExclusive)
		.setInitialLayout(use_staging ? vk::ImageLayout::eUndefined : vk::ImageLayout::ePreinitialized);

	_textureImage = device.createImage(image_create_info);

	vk::MemoryRequirements memory_requirements   = device.getImageMemoryRequirements(_textureImage);
	uint32_t memory_type           = engine::findMemoryType(physicalDevice.getMemoryProperties(), memory_requirements.memoryTypeBits, 
		use_staging ? vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent : vk::MemoryPropertyFlagBits::eDeviceLocal);
	vk::MemoryAllocateInfo memory_allocate_info  = vk::MemoryAllocateInfo()
		.setAllocationSize(memory_requirements.size)
		.setMemoryTypeIndex(memory_type);
	_textureImageMemory = device.allocateMemory(memory_allocate_info);
	device.bindImageMemory(_textureImage, _textureImageMemory, 0);

	if (use_staging) {
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

		// Setup buffer copy regions for each mip level
		std::vector<vk::BufferImageCopy> buffer_copy_regions(mip_levels);
		for (uint32_t i = 0; i < mip_levels; i++)
		{
			uint32_t offset = getOffset(i, 0, 0);
			
			buffer_copy_regions[i].imageSubresource.aspectMask     = vk::ImageAspectFlagBits::eColor;
			buffer_copy_regions[i].imageSubresource.mipLevel       = i;
			buffer_copy_regions[i].imageSubresource.baseArrayLayer = 0;
			buffer_copy_regions[i].imageSubresource.layerCount     = 1;
			buffer_copy_regions[i].imageExtent.width               = extent.width >> i;
			buffer_copy_regions[i].imageExtent.height              = extent.height >> i;
			buffer_copy_regions[i].imageExtent.depth               = 1;
			buffer_copy_regions[i].bufferOffset                    = offset;
		}

		transitionImageLayout(oneTimeCommandSubmitter, _textureImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

		oneTimeCommandSubmitter.submit([&] (const vk::CommandBuffer& commandBuffer) {
			// Copy mip levels from staging buffer
			stagingBuffer.copyTo(commandBuffer, _textureImage, buffer_copy_regions);
		});

		transitionImageLayout(oneTimeCommandSubmitter, _textureImage, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
	} else {
		void *data = device.mapMemory(_textureImageMemory, 0, memory_requirements.size);
		// Copy image data of the first mip level into memory
		//memcpy(data, data, ktxTexture_GetImageSize(ktx_texture, 0));
		memcpy(data, data, getImageSize(0));
		device.unmapMemory(_textureImageMemory);

		transitionImageLayout(oneTimeCommandSubmitter, _textureImage, vk::ImageLayout::ePreinitialized, vk::ImageLayout::eShaderReadOnlyOptimal);
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

	const vk::FormatProperties fmt_props = physicalDevice.getFormatProperties(format);

	bool has_linear_filter = !!(fmt_props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear);

	vk::SamplerCreateInfo sampler_create_info = vk::SamplerCreateInfo()
		.setMagFilter(has_linear_filter ? vk::Filter::eLinear : vk::Filter::eNearest)
		.setMinFilter(has_linear_filter ? vk::Filter::eLinear : vk::Filter::eNearest)
		.setMipmapMode(has_linear_filter ? vk::SamplerMipmapMode::eLinear : vk::SamplerMipmapMode::eNearest)
		.setAddressModeU(vk::SamplerAddressMode::eClampToEdge)
		.setAddressModeV(vk::SamplerAddressMode::eClampToEdge)
		.setAddressModeW(vk::SamplerAddressMode::eClampToEdge)
		.setMaxAnisotropy(1.0f < maxAnisotropy)
		.setCompareOp(vk::CompareOp::eNever)
		.setMinLod(0.0f)
		.setMaxLod((use_staging) ? static_cast<float>(mip_levels) : 0.0f)
		.setBorderColor(vk::BorderColor::eFloatOpaqueWhite);

	_textureSampler = _device.createSampler(sampler_create_info);
	// Create image view
	// Textures are not directly accessed by the shaders and
	// are abstracted by image views containing additional
	// information and sub resource ranges
	vk::ImageViewCreateInfo image_view_create_info;
	image_view_create_info.viewType   = vk::ImageViewType::e2D;
	image_view_create_info.format     = format;
	image_view_create_info.components = {vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA};
	// The subresource range describes the set of mip levels (and array layers) that can be accessed through this image image_view_create_info
	// It's possible to create multiple image views for a single image referring to different (and/or overlapping) ranges of the image
	image_view_create_info.subresourceRange.aspectMask     = vk::ImageAspectFlagBits::eColor;
	image_view_create_info.subresourceRange.baseMipLevel   = 0;
	image_view_create_info.subresourceRange.baseArrayLayer = 0;
	image_view_create_info.subresourceRange.layerCount     = 1;
	// Linear tiling usually won't support mip maps
	// Only set mip map count if optimal tiling is used
	image_view_create_info.subresourceRange.levelCount = (use_staging) ? mip_levels : 1;
	// The image_view_create_info will be based on the texture's image
	image_view_create_info.image = _textureImage;
	_textureImageView          = device.createImageView(image_view_create_info);
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

void Image::transitionImageLayout(engine::OneTimeCommandSubmitter& oneTimeCommandSubmitter,
    vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t levelCount) {
  vk::ImageSubresourceRange subresource = vk::ImageSubresourceRange()
      .setAspectMask(vk::ImageAspectFlagBits::eColor)
      .setBaseMipLevel(0)
      .setLevelCount(levelCount)
      .setLayerCount(1);
  
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