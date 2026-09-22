#pragma once

#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <glm/glm.hpp>

#include <fstream>
#include <limits>

namespace engine {
struct GlobalUbo {
  glm::mat4 projection{1.f};
  glm::mat4 view{1.f};
  glm::mat4 inverseView{1.f};
  glm::vec4 ambientLightColor{1.f, 1.f, 1.f, .02f};  // w is intensity
  //PointLight pointLights[MAX_LIGHTS];
  //int numLights;
};

struct SimplePushConstantData {
  glm::mat4 modelMatrix{1.f};
  glm::mat4 normalMatrix{1.f};
};

vk::AccessFlags getAccessFlags(vk::ImageLayout layout);
vk::PipelineStageFlags getPipelineStageFlags(vk::ImageLayout layout);


uint32_t findMemoryType(
    vk::PhysicalDeviceMemoryProperties const& memoryProperties,
    uint32_t typeBits,
    vk::MemoryPropertyFlags requirementsMask);

void setImageLayout(
    vk::CommandBuffer const& commandBuffer,
    vk::Image image,
    vk::PipelineStageFlags sourceStageMask,
    vk::PipelineStageFlags destinationStageMask,
    vk::AccessFlags sourceAccessMask,
    vk::AccessFlags destinationAccessMask,
    vk::ImageLayout oldImageLayout,
    vk::ImageLayout newImageLayout,
    vk::ImageSubresourceRange const& subresourceRange);

void setImageLayout(
    vk::CommandBuffer const& commandBuffer,
    vk::Image image,
    vk::Format format,
    vk::ImageLayout oldImageLayout,
    vk::ImageLayout newImageLayout);

void setImageLayout(
    vk::CommandBuffer const& commandBuffer,
    vk::Image image,
    vk::ImageLayout oldImageLayout,
    vk::ImageLayout newImageLayout,
    vk::ImageSubresourceRange const& subresourceRange
);


void copyBufferToImage(
    vk::CommandBuffer const& commandBuffer,
    vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height, uint32_t layerCount);


/*
void image_layout_transition(VkCommandBuffer                command_buffer,
                             VkImage                        image,
                             VkImageLayout                  old_layout,
                             VkImageLayout                  new_layout,
                             VkImageSubresourceRange const &subresource_range);

void image_layout_transition(VkCommandBuffer                                                 command_buffer,
                             std::vector<std::pair<VkImage, VkImageSubresourceRange>> const &imagesAndRanges,
                             VkImageLayout                                                   old_layout,
                             VkImageLayout                                                   new_layout);
*/

vk::SurfaceFormatKHR pickSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const& formats);
vk::PresentModeKHR pickPresentMode(std::vector<vk::PresentModeKHR> const& presentModes);
uint32_t clampSurfaceImageCount(
    const uint32_t desiredImageCount, const uint32_t minImageCount, const uint32_t maxImageCount);
std::pair<uint32_t, uint32_t>    findGraphicsAndPresentQueueFamilyIndex( vk::PhysicalDevice physicalDevice, vk::SurfaceKHR const & surface );

std::string readFileString(const std::string& filepath);
}  // namespace engine
