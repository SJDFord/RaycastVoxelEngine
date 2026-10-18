#pragma once 

#include "game_object.hpp"
#include "../../logic/camera.hpp"

#include "vulkan/vulkan.hpp"

namespace engine {
struct FrameInfo {
    int frameIndex;
    float frameTime;
    vk::CommandBuffer commandBuffer;
    Camera &camera;
    vk::DescriptorSet globalDescriptorSet;
    GameObject::Map &gameObjects;
};
}