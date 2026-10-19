#include "utils.hpp"

namespace engine {

void copyBufferToImage(
    vk::CommandBuffer const& commandBuffer,
    vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height, uint32_t layerCount) {
  vk::BufferImageCopy region{};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;

  region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = layerCount;


  region.imageOffset = vk::Offset3D(0, 0, 0);
  region.imageExtent = vk::Extent3D(width, height, 1);

  commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, region);
}

vk::PipelineStageFlags getPipelineStageFlags(vk::ImageLayout layout)
{
	switch (layout)
	{
		case vk::ImageLayout::eUndefined:
      return vk::PipelineStageFlagBits::eTopOfPipe;
    case vk::ImageLayout::ePreinitialized:
      return vk::PipelineStageFlagBits::eHost;
    case vk::ImageLayout::eTransferDstOptimal:
    case vk::ImageLayout::eTransferSrcOptimal:
      return vk::PipelineStageFlagBits::eTransfer;
    case vk::ImageLayout::eColorAttachmentOptimal:
      return vk::PipelineStageFlagBits::eColorAttachmentOutput;
    case vk::ImageLayout::eDepthAttachmentOptimal:
      return vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests;
    case vk::ImageLayout::eFragmentShadingRateAttachmentOptimalKHR:
      return vk::PipelineStageFlagBits::eFragmentShadingRateAttachmentKHR;
    case vk::ImageLayout::eShaderReadOnlyOptimal:
      return vk::PipelineStageFlagBits::eVertexShader | vk::PipelineStageFlagBits::eFragmentShader;
    case vk::ImageLayout::ePresentSrcKHR:
      return vk::PipelineStageFlagBits::eBottomOfPipe;
    case vk::ImageLayout::eGeneral:
			assert(false && "Don't know how to get a meaningful VkPipelineStageFlags for VK_IMAGE_LAYOUT_GENERAL! Don't use it!");
			return vk::PipelineStageFlagBits::eNone;
		default:
			assert(false);
			return vk::PipelineStageFlagBits::eNone;
	}
}

vk::AccessFlags getAccessFlags(vk::ImageLayout layout)
{
	switch (layout)
	{
    case vk::ImageLayout::eUndefined:
    case vk::ImageLayout::ePresentSrcKHR:
      return vk::AccessFlagBits::eNone;
    case vk::ImageLayout::ePreinitialized:
      return vk::AccessFlagBits::eHostWrite;
    case vk::ImageLayout::eColorAttachmentOptimal:
      return vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
    case vk::ImageLayout::eDepthAttachmentOptimal:
      return vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
    case vk::ImageLayout::eFragmentShadingRateAttachmentOptimalKHR:
      return vk::AccessFlagBits::eFragmentShadingRateAttachmentReadKHR;
    case vk::ImageLayout::eShaderReadOnlyOptimal:
      return vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eInputAttachmentRead;
    case vk::ImageLayout::eTransferSrcOptimal:
      return vk::AccessFlagBits::eTransferRead;
    case vk::ImageLayout::eTransferDstOptimal:
      return vk::AccessFlagBits::eTransferWrite;
    case vk::ImageLayout::eGeneral:
			assert(false && "Don't know how to get a meaningful VkAccessFlags for VK_IMAGE_LAYOUT_GENERAL! Don't use it!");
      return vk::AccessFlagBits::eNone;
		default:
			assert(false);
      return vk::AccessFlagBits::eNone;
	}
}

uint32_t findMemoryType(
    vk::PhysicalDeviceMemoryProperties const& memoryProperties,
    uint32_t typeBits,
    vk::MemoryPropertyFlags requirementsMask) {
  uint32_t typeIndex = uint32_t(~0);
  for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
    if ((typeBits & 1) &&
        ((memoryProperties.memoryTypes[i].propertyFlags & requirementsMask) == requirementsMask)) {
      typeIndex = i;
      break;
    }
    typeBits >>= 1;
  }
  assert(typeIndex != uint32_t(~0));
  return typeIndex;
}

std::pair<uint32_t, uint32_t> findGraphicsAndPresentQueueFamilyIndex(
    vk::PhysicalDevice physicalDevice, vk::SurfaceKHR const& surface) {
  std::vector<vk::QueueFamilyProperties> queueFamilyProperties =
      physicalDevice.getQueueFamilyProperties();
  assert(queueFamilyProperties.size() < (std::numeric_limits<uint32_t>::max)());

  // look for a queueFamilyIndex that supports graphics and present
  auto combinedIt = std::find_if(
      queueFamilyProperties.begin(),
      queueFamilyProperties.end(),
      [&physicalDevice, &surface](vk::QueueFamilyProperties const& qfp) {
        static uint32_t index = 0;
        return (qfp.queueFlags & vk::QueueFlagBits::eGraphics) &&
               physicalDevice.getSurfaceSupportKHR(index++, surface);
      });

  if (combinedIt != queueFamilyProperties.end()) {
    uint32_t index =
        static_cast<uint32_t>(std::distance(queueFamilyProperties.begin(), combinedIt));
    return {index, index};  // the first index that supports graphics and present
  }

  // there's no single index that supports both graphics and present -> look for separate ones
  auto graphicsIt = std::find_if(
      queueFamilyProperties.begin(),
      queueFamilyProperties.end(),
      [](vk::QueueFamilyProperties const& qfp) {
        return qfp.queueFlags & vk::QueueFlagBits::eGraphics;
      });

  if (graphicsIt == queueFamilyProperties.end()) {
    throw std::runtime_error(
        "Could not find a queue family index that supports graphics -> terminating");
  }

  uint32_t graphicsIndex =
      static_cast<uint32_t>(std::distance(queueFamilyProperties.begin(), graphicsIt));
  auto presentIt = std::find_if(
      queueFamilyProperties.begin(),
      queueFamilyProperties.end(),
      [&physicalDevice, &surface](vk::QueueFamilyProperties const&) {
        static uint32_t index = 0;
        return physicalDevice.getSurfaceSupportKHR(index++, surface);
      });

  if (presentIt != queueFamilyProperties.end()) {
    uint32_t presentIndex =
        static_cast<uint32_t>(std::distance(queueFamilyProperties.begin(), presentIt));
    return {graphicsIndex, presentIndex};
  }

  throw std::runtime_error(
      "Could not find a queue family index that supports present -> terminating");
}

std::string readFileString(const std::string& filepath) {
  std::string enginePath = filepath;
  std::ifstream file{enginePath, std::ios::ate | std::ios::binary};

  if (!file.is_open()) {
    throw std::runtime_error("failed to open file: " + enginePath);
  }

  size_t fileSize = static_cast<size_t>(file.tellg());
  std::string buffer(fileSize, ' ');

  file.seekg(0);
  file.read(&buffer[0], fileSize);

  file.close();
  return buffer;
}

}  // namespace engine

