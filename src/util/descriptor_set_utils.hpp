#pragma once

#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_raii.hpp"
#include "../engine/texture.hpp"


namespace DescriptorSetUtils {
          void                 updateDescriptorSets( vk::Device const &                                                                                              device,
                                               vk::DescriptorSet const &                                                                                       descriptorSet,
                                               std::vector<std::tuple<vk::DescriptorType, vk::Buffer const &, vk::DeviceSize, vk::BufferView const &>> const & bufferData,
                                               engine::Texture const &                                                                                     textureData,
                                               uint32_t bindingOffset = 0 );
    void                 updateDescriptorSets( vk::Device const &                                                                                              device,
                                               vk::DescriptorSet const &                                                                                       descriptorSet,
                                               std::vector<std::tuple<vk::DescriptorType, vk::Buffer const &, vk::DeviceSize, vk::BufferView const &>> const & bufferData,
                                               std::vector<engine::Texture> const &                                                                        textureData,
                                               uint32_t bindingOffset = 0 );

}
