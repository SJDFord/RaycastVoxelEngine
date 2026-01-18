#include "app.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <chrono>
#include <stdexcept>


App::App() {   
	window = std::make_shared<Window>(WIDTH, HEIGHT, "Vulkan RAII Voxel Engine");
	device = std::make_shared<Device>(*window.get());

    initFrames();
}

App::~App() {}

void App::run() {
    while (!window->shouldClose()) {
        window->pollEvents();

        if (window->isKeyPressed(KeyboardKey::ESCAPE)) {
        window->close();
        }
    }
    device->waitIdle();
}

void App::initFrames() {
    vk::CommandBufferAllocateInfo commandBufferAllocateInfo{};
    commandBufferAllocateInfo.commandPool = device->getCommandPool();
    commandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
    commandBufferAllocateInfo.commandBufferCount = IN_FLIGHT_FRAME_COUNT;
    auto commandBuffers{device->device().allocateCommandBuffers(commandBufferAllocateInfo)};

    for (size_t i = 0; i < IN_FLIGHT_FRAME_COUNT; i++) {
        frames[i].emplace(
            Frame{
            std::move(commandBuffers[i]),
             vk::raii::Semaphore{device->device(), vk::SemaphoreCreateInfo{}},
             vk::raii::Semaphore{device->device(), vk::SemaphoreCreateInfo{}},
             vk::raii::Fence{
      device->device(), vk::FenceCreateInfo { vk::FenceCreateFlagBits::eSignaled }}
    });
    }
}