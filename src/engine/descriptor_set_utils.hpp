#pragma once

#include "texture.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_raii.hpp"

namespace engine {

void updateDescriptorSets(
    vk::Device const &device,
    vk::DescriptorSet const &descriptorSet,
    std::vector<std::tuple<
        vk::DescriptorType,
        vk::Buffer const &,
        vk::DeviceSize,
        vk::BufferView const &>> const &bufferData,
    Texture const &textureData,
    uint32_t bindingOffset = 0);
void updateDescriptorSets(
    vk::Device const &device,
    vk::DescriptorSet const &descriptorSet,
    std::vector<std::tuple<
        vk::DescriptorType,
        vk::Buffer const &,
        vk::DeviceSize,
        vk::BufferView const &>> const &bufferData,
    std::vector<Texture> const &textureData,
    uint32_t bindingOffset = 0);
}  // namespace engine
