#include "descriptor_set_utils.hpp"

namespace engine {

void updateDescriptorSets(
    vk::Device const &device,
    vk::DescriptorSet const &descriptorSet,
    std::vector<std::tuple<
        vk::DescriptorType,
        vk::Buffer const &,
        vk::DeviceSize,
        vk::BufferView const &>> const &bufferData,
    Texture const &textureData,
    uint32_t bindingOffset) {
  std::vector<vk::DescriptorBufferInfo> bufferInfos;
  bufferInfos.reserve(bufferData.size());

  std::vector<vk::WriteDescriptorSet> writeDescriptorSets;
  writeDescriptorSets.reserve(bufferData.size() + 1);
  uint32_t dstBinding = bindingOffset;
  for (auto const &bd : bufferData) {
    bufferInfos.emplace_back(std::get<1>(bd), 0, std::get<2>(bd));
    writeDescriptorSets.emplace_back(
        descriptorSet,
        dstBinding++,
        0,
        1,
        std::get<0>(bd),
        nullptr,
        &bufferInfos.back(),
        &std::get<3>(bd));
  }

  vk::DescriptorImageInfo imageInfo(
      textureData.getSampler(),
      textureData.getImage()->getImageView(),
      vk::ImageLayout::eShaderReadOnlyOptimal);
  writeDescriptorSets.emplace_back(
      descriptorSet,
      dstBinding,
      0,
      vk::DescriptorType::eCombinedImageSampler,
      imageInfo,
      nullptr,
      nullptr);

  device.updateDescriptorSets(writeDescriptorSets, nullptr);
}

void updateDescriptorSets(
    vk::Device const &device,
    vk::DescriptorSet const &descriptorSet,
    std::vector<std::tuple<
        vk::DescriptorType,
        vk::Buffer const &,
        vk::DeviceSize,
        vk::BufferView const &>> const &bufferData,
    std::vector<Texture> const &textureData,
    uint32_t bindingOffset) {
  std::vector<vk::DescriptorBufferInfo> bufferInfos;
  bufferInfos.reserve(bufferData.size());

  std::vector<vk::WriteDescriptorSet> writeDescriptorSets;
  writeDescriptorSets.reserve(bufferData.size() + (textureData.empty() ? 0 : 1));
  uint32_t dstBinding = bindingOffset;
  for (auto const &bd : bufferData) {
    bufferInfos.emplace_back(std::get<1>(bd), 0, std::get<2>(bd));
    writeDescriptorSets.emplace_back(
        descriptorSet,
        dstBinding++,
        0,
        1,
        std::get<0>(bd),
        nullptr,
        &bufferInfos.back(),
        &std::get<3>(bd));
  }

  std::vector<vk::DescriptorImageInfo> imageInfos;
  if (!textureData.empty()) {
    imageInfos.reserve(textureData.size());
    for (auto const &td : textureData) {
      imageInfos.emplace_back(
          td.getSampler(),
          td.getImage()->getImageView(),
          vk::ImageLayout::eShaderReadOnlyOptimal);
    }
    writeDescriptorSets.emplace_back(
        descriptorSet,
        dstBinding,
        0,
        static_cast<uint32_t>(imageInfos.size()),
        vk::DescriptorType::eCombinedImageSampler,
        imageInfos.data(),
        nullptr,
        nullptr);
  }

  device.updateDescriptorSets(writeDescriptorSets, nullptr);
}

}  // namespace engine

