#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "physical_device.hpp"
#include "queue_family_indices.hpp"

#include <memory>
#include <set>
#include <iostream>

class LogicalDevice {
public:
        #ifdef NDEBUG
        const bool enableValidationLayers = false;
        #else
        const bool enableValidationLayers = true;
        #endif



        LogicalDevice(std::shared_ptr<PhysicalDevice> physicalDevice, VkSurfaceKHR surface);
        ~LogicalDevice();

        LogicalDevice(const LogicalDevice&) = delete;
        LogicalDevice& operator=(const LogicalDevice&) = delete;

        void createCommandPool(const VkCommandPoolCreateInfo* createInfo, VkCommandPool* commandPool);
        void createBuffer(
            VkDeviceSize size,
            VkBufferUsageFlags usage,
            uint32_t memoryTypeIndex,
            VkBuffer &buffer,
            VkDeviceMemory &bufferMemory);
        VkCommandBuffer beginSingleTimeCommands(VkCommandPool commandPool);
        void endSingleTimeCommands(VkCommandBuffer commandBuffer, VkCommandPool commandPool);
        void createImageWithInfo(
            const VkImageCreateInfo &imageInfo,
            uint32_t memoryTypeIndex,
            VkImage &image,
            VkDeviceMemory &imageMemory);
private:
    VkDeviceCreateInfo createInfo;
    VkDevice device;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
    const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
};