#pragma once

#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_raii.hpp"


namespace VulkanUtils {

    uint32_t findMemoryType( 
        vk::PhysicalDeviceMemoryProperties const & memoryProperties, 
        uint32_t typeBits, 
        vk::MemoryPropertyFlags requirementsMask 
    );

}
