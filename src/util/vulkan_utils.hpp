#pragma once

#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_raii.hpp"


namespace VulkanUtils {

    uint32_t findMemoryType( 
        vk::PhysicalDeviceMemoryProperties const & memoryProperties, 
        uint32_t typeBits, 
        vk::MemoryPropertyFlags requirementsMask 
    );
    
    void setImageLayout(
      vk::CommandBuffer const & commandBuffer, vk::Image image, vk::Format format, vk::ImageLayout oldImageLayout, vk::ImageLayout newImageLayout );

}
