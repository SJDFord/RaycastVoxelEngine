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

    vk::SurfaceFormatKHR pickSurfaceFormat( std::vector<vk::SurfaceFormatKHR> const & formats );
    vk::PresentModeKHR pickPresentMode( std::vector<vk::PresentModeKHR> const & presentModes );
    uint32_t clampSurfaceImageCount( const uint32_t desiredImageCount, const uint32_t minImageCount, const uint32_t maxImageCount );
}
