#include "one_time_command_submitter.hpp"

namespace engine {

OneTimeCommandSubmitter::OneTimeCommandSubmitter(
    vk::Device device, 
    vk::CommandPool commandPool, 
    uint32_t graphicsFamilyIndex): _device{device}, _commandPool{commandPool}, _graphicsFamilyIndex{graphicsFamilyIndex} {

}


OneTimeCommandSubmitter::~OneTimeCommandSubmitter() {

}


void OneTimeCommandSubmitter::submit(std::function<void(vk::CommandBuffer const &)> commandBufferOperation) {
  vk::CommandBufferAllocateInfo allocInfo = vk::CommandBufferAllocateInfo()
    .setLevel(vk::CommandBufferLevel::ePrimary)
    .setCommandPool(_commandPool)
    .setCommandBufferCount(1);


  auto commandBuffers = _device.allocateCommandBuffers(allocInfo);
  auto commandBuffer = commandBuffers.front();

  commandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
  commandBufferOperation(commandBuffer);
  commandBuffer.end();
  vk::SubmitInfo submitInfo = vk::SubmitInfo().setCommandBuffers(commandBuffer);

  auto queue = _device.getQueue(_graphicsFamilyIndex, 0);
  queue.submit(submitInfo, VK_NULL_HANDLE);
  queue.waitIdle();

  _device.freeCommandBuffers(_commandPool, commandBuffer);
}

}