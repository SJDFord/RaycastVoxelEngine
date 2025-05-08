#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include "queue_family_indices.hpp"
#include "swap_chain_support_details.hpp"

class PhysicalDevice {
public:
        PhysicalDevice(const VkInstance& instance);
        ~PhysicalDevice();

        PhysicalDevice(const PhysicalDevice&) = delete;
        PhysicalDevice& operator=(const PhysicalDevice&) = delete;

        QueueFamilyIndices findQueueFamilies(VkSurfaceKHR surface);
        SwapChainSupportDetails querySwapChainSupport(VkSurfaceKHR surface);
        VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

        void createLogicalDevice(const VkDeviceCreateInfo* createInfo, VkDevice* pDevice);
private:
        void ReadPhysicalDevices(const VkInstance& instance);
        int32_t ScoreDevice(const VkPhysicalDevice& device);
        VkPhysicalDevice GetBestAvailPhysicalDevice();
        void getAttributes();
        void retrieveQueueFamilyProperties();
        //bool isDeviceSuitable(VkPhysicalDevice device);
        //QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkPhysicalDeviceProperties properties;
        VkPhysicalDeviceFeatures features;
        VkPhysicalDeviceMemoryProperties memoryProperties;
        std::vector<VkQueueFamilyProperties> queueFamilyProperties;

};