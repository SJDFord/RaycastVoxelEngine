#pragma once

#include "lve_window.hpp"
#include <vector>

class PhysicalDevice {
public:
        PhysicalDevice(const VkInstance& instance);
        ~PhysicalDevice();

        PhysicalDevice(const PhysicalDevice&) = delete;
        PhysicalDevice& operator=(const PhysicalDevice&) = delete;

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