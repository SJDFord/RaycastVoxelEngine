/*
#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "queue_family_indices.hpp"

class CommandPool {
public:
        CommandPool(const VkInstance& instance, const PhysicalDevice& physicalDevice);
        ~CommandPool();

        CommandPool(const CommandPool&) = delete;
        CommandPool& operator=(const CommandPool&) = delete;
private:
    VkDeviceCreateInfo createInfo;

    QueueFamilyIndices findQueueFamilies(const PhysicalDevice& physicalDevice);
};
*/