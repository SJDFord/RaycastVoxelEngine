#pragma once

#include "vulkan/vulkan.hpp"
#include <glm/glm.hpp>

#include <fstream>
#include <limits>

namespace engine {

vk::AccessFlags getAccessFlags(vk::ImageLayout layout);
vk::PipelineStageFlags getPipelineStageFlags(vk::ImageLayout layout);

uint32_t findMemoryType(
    vk::PhysicalDeviceMemoryProperties const& memoryProperties,
    uint32_t typeBits,
    vk::MemoryPropertyFlags requirementsMask);

std::pair<uint32_t, uint32_t>    findGraphicsAndPresentQueueFamilyIndex( vk::PhysicalDevice physicalDevice, vk::SurfaceKHR const & surface );

std::string readFileString(const std::string& filepath);
}  // namespace engine
