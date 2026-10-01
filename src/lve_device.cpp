#include "lve_device.hpp"

// std headers
#include <cstring>
#include <iostream>
#include <set>
#include <unordered_set>
#include "./engine/instance_builder.hpp"
#include "./engine/utils.hpp"

#include "./engine/physical_device_strategy.hpp"
#include "./engine/ranked_physical_device_strategy.hpp"
#include "./engine/device_builder.hpp"

namespace lve {

// class member functions
LveDevice::LveDevice(engine::Window &window) : _window{window} {
  createInstance();
  createSurface();
  auto physicalDevices = _instance.enumeratePhysicalDevices();
  _physicalDevice = engine::RankedPhysicalDeviceStrategy().pickPhysicalDevice(physicalDevices);

  createLogicalDevice();
  createCommandPool();
}

LveDevice::~LveDevice() {
  _device.destroyCommandPool(_commandPool);
  vkDestroyDevice(_device, nullptr);
  vkDestroySurfaceKHR(_instance, _surface, nullptr);
  vkDestroyInstance(_instance, nullptr);
}

void LveDevice::createInstance() {
  std::vector<std::string> extensions = _window.getRequiredExtensions();
  _instance = engine::InstanceBuilder("Voxel App", "RaycastVoxelEngine", VK_API_VERSION_1_3)
                              .setExtensions(extensions)
                              .build();
  _window.createSurface(_instance);
}

void LveDevice::createLogicalDevice() {
  QueueFamilyIndices indices = findQueueFamilies(_surface);

  _device = engine::DeviceBuilder(_physicalDevice, indices.graphicsFamily)
    .setExtensions({
        VK_KHR_SWAPCHAIN_EXTENSION_NAME, 
        //VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME
    })
    //.setPNext(new vk::PhysicalDeviceDynamicRenderingFeatures(VK_TRUE))
    .build();

  _graphicsQueue = _device.getQueue(indices.graphicsFamily, 0);
  _presentQueue = _device.getQueue(indices.presentFamily, 0);
}

void LveDevice::createCommandPool() {
  QueueFamilyIndices queueFamilyIndices = findPhysicalQueueFamilies();
  _commandPool = _device.createCommandPool({
    vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer, 
    queueFamilyIndices.graphicsFamily
  });
}

void LveDevice::createSurface() { _surface = _window.getSurface(); }

SwapChainSupportDetails LveDevice::querySwapChainSupport() {
  SwapChainSupportDetails details;
  details.capabilities = _physicalDevice.getSurfaceCapabilitiesKHR(_surface);
  details.formats = _physicalDevice.getSurfaceFormatsKHR(_surface);
  details.presentModes = _physicalDevice.getSurfacePresentModesKHR(_surface);
  return details;
}

uint32_t LveDevice::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {

  auto memoryProperties = _physicalDevice.getMemoryProperties();
  for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) &&
        (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }

  throw std::runtime_error("failed to find suitable memory type!");
}

void LveDevice::createBuffer(
    vk::DeviceSize size,
    vk::BufferUsageFlags usage,
    vk::MemoryPropertyFlags properties,
    vk::Buffer &buffer,
    vk::DeviceMemory &bufferMemory) {
  vk::BufferCreateInfo bufferInfo = vk::BufferCreateInfo()
    .setSize(size)
    .setUsage(usage)
    .setSharingMode(vk::SharingMode::eExclusive);

  buffer = _device.createBuffer(bufferInfo);
  auto memoryRequirements = _device.getBufferMemoryRequirements(buffer);

  vk::MemoryAllocateInfo allocInfo = vk::MemoryAllocateInfo()
    .setAllocationSize(memoryRequirements.size)
    .setMemoryTypeIndex(findMemoryType(memoryRequirements.memoryTypeBits, properties));

  bufferMemory = _device.allocateMemory(allocInfo);
  _device.bindBufferMemory(buffer, bufferMemory, 0);
}

vk::CommandBuffer LveDevice::beginSingleTimeCommands() {
  vk::CommandBufferAllocateInfo allocInfo = vk::CommandBufferAllocateInfo()
    .setLevel(vk::CommandBufferLevel::ePrimary)
    .setCommandPool(_commandPool)
    .setCommandBufferCount(1);


  auto commandBuffers = _device.allocateCommandBuffers(allocInfo);
  auto commandBuffer = commandBuffers.front();

  commandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
  return commandBuffer;
}

void LveDevice::endSingleTimeCommands(vk::CommandBuffer commandBuffer) {
  commandBuffer.end();
  vk::SubmitInfo submitInfo = vk::SubmitInfo().setCommandBuffers(commandBuffer);

  _graphicsQueue.submit(submitInfo, VK_NULL_HANDLE);
  _graphicsQueue.waitIdle();

  _device.freeCommandBuffers(_commandPool, commandBuffer);
}

void LveDevice::copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size) {
  vk::CommandBuffer commandBuffer = beginSingleTimeCommands();

  vk::BufferCopy copyRegion = vk::BufferCopy()
    .setSrcOffset(0)
    .setDstOffset(0)
    .setSize(size);
  commandBuffer.copyBuffer(srcBuffer, dstBuffer, copyRegion);
  endSingleTimeCommands(commandBuffer);
}

void LveDevice::copyBufferToImage(
    vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height, uint32_t layerCount) {
  vk::CommandBuffer commandBuffer = beginSingleTimeCommands();

  vk::ImageSubresourceLayers subresource = vk::ImageSubresourceLayers()
      .setAspectMask(vk::ImageAspectFlagBits::eColor)
      .setMipLevel(0)
      .setBaseArrayLayer(0)
      .setLayerCount(layerCount);
  vk::BufferImageCopy region = vk::BufferImageCopy()
    .setBufferOffset(0)
    .setBufferRowLength(0)
    .setBufferImageHeight(0)
    .setImageSubresource(subresource)
    .setImageOffset({0, 0, 0})
    .setImageExtent({width, height, 1});
  
  commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, region);
  endSingleTimeCommands(commandBuffer);
}

void LveDevice::transitionImageLayout(
    vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) {
  vk::CommandBuffer commandBuffer = beginSingleTimeCommands();
  vk::ImageSubresourceRange subresource = vk::ImageSubresourceRange()
      .setAspectMask(vk::ImageAspectFlagBits::eColor)
      .setBaseMipLevel(0)
      .setLevelCount(1)
      .setBaseArrayLayer(0)
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

    sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
    destinationStage = vk::PipelineStageFlagBits::eTransfer;
  } else if (
      oldLayout == vk::ImageLayout::eTransferDstOptimal &&
      newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
    barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;;

    sourceStage = vk::PipelineStageFlagBits::eTransfer;
    destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
  } else {
    throw std::invalid_argument("unsupported layout transition!");
  }

  commandBuffer.pipelineBarrier(sourceStage, destinationStage, vk::DependencyFlags(), {}, {}, barrier);
  endSingleTimeCommands(commandBuffer);
}

void LveDevice::createImageWithInfo(
      const vk::ImageCreateInfo &imageInfo,
      vk::MemoryPropertyFlags properties,
      vk::Image &image,
      vk::DeviceMemory &imageMemory) {
  image = _device.createImage(imageInfo);
  vk::MemoryRequirements memRequirements = _device.getImageMemoryRequirements(image);

  vk::MemoryAllocateInfo allocInfo = vk::MemoryAllocateInfo()
    .setAllocationSize(memRequirements.size)
    .setMemoryTypeIndex(findMemoryType(memRequirements.memoryTypeBits, properties));

  imageMemory = _device.allocateMemory(allocInfo);

  _device.bindImageMemory(image, imageMemory, 0);
}

vk::ImageView LveDevice::createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags flags) {
  vk::ImageSubresourceRange subresourceRange = vk::ImageSubresourceRange(flags)
    .setBaseMipLevel(0)
    .setLevelCount(1)
    .setBaseArrayLayer(0)
    .setLayerCount(1);

  vk::ImageViewCreateInfo viewInfo = vk::ImageViewCreateInfo()
    .setImage(image)
    .setViewType(vk::ImageViewType::e2D)
    .setFormat(format)
    .setSubresourceRange(subresourceRange);

  vk::ImageView imageView = _device.createImageView(viewInfo);

  return imageView;
}

vk::Sampler LveDevice::createSampler() { 
    vk::SamplerCreateInfo samplerInfo = vk::SamplerCreateInfo(vk::SamplerCreateFlags())
      .setMagFilter(vk::Filter::eLinear)
      .setMinFilter(vk::Filter::eLinear)
      .setAddressModeU(vk::SamplerAddressMode::eRepeat)
      .setAddressModeV(vk::SamplerAddressMode::eRepeat)
      .setAddressModeW(vk::SamplerAddressMode::eRepeat)
      .setAnisotropyEnable(false)
      .setMaxAnisotropy(1.0f)
      .setBorderColor(vk::BorderColor::eIntOpaqueBlack)
      .setUnnormalizedCoordinates(false)
      .setCompareEnable(false)
      .setCompareOp(vk::CompareOp::eAlways)
      .setMipmapMode(vk::SamplerMipmapMode::eLinear)
      .setMipLodBias(0.0f)
      .setMinLod(0.0f)
      .setMaxLod(0.0f);

    vk::Sampler sampler = _device.createSampler(samplerInfo);
    return sampler;
}

void LveDevice::waitIdle() { vkDeviceWaitIdle(_device); }

void LveDevice::destroyShaderModule(VkShaderModule shaderModule) {
    vkDestroyShaderModule(_device, shaderModule, nullptr);
}
void LveDevice::destroyPipeline(VkPipeline pipeline) {
    vkDestroyPipeline(_device, pipeline, nullptr);
}

void LveDevice::destroyPipelineLayout(VkPipelineLayout pipelineLayout) { 
  vkDestroyPipelineLayout(_device, pipelineLayout, nullptr);
}

void LveDevice::destroySampler(VkSampler sampler) {
    vkDestroySampler(_device, sampler, nullptr);
}

void LveDevice::destroyImageView(VkImageView imageView) {
  vkDestroyImageView(_device, imageView, nullptr);
}

void LveDevice::destroyImage(VkImage image) {
  vkDestroyImage(_device, image, nullptr);
}
void LveDevice::freeMemory(VkDeviceMemory deviceMemory) { 
    vkFreeMemory(_device, deviceMemory, nullptr); 
}

QueueFamilyIndices LveDevice::findQueueFamilies(vk::SurfaceKHR surface) {
  QueueFamilyIndices indices;

  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyCount, nullptr);

  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyCount, queueFamilies.data());

  int i = 0;
  for (const auto &queueFamily : queueFamilies) {
    if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphicsFamily = i;
      indices.graphicsFamilyHasValue = true;
    }
    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(_physicalDevice, i, surface, &presentSupport);
    if (queueFamily.queueCount > 0 && presentSupport) {
      indices.presentFamily = i;
      indices.presentFamilyHasValue = true;
    }
    if (indices.isComplete()) {
      break;
    }

    i++;
  }

  return indices;
}

}  // namespace lve
