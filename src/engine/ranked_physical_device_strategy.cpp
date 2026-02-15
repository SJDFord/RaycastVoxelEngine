
#include "ranked_physical_device_strategy.hpp"

namespace engine {


vk::PhysicalDevice RankedPhysicalDeviceStrategy::pickPhysicalDevice(
   const std::vector<vk::PhysicalDevice>& physicalDevices) {
   std::cout << "TEst!!!" << std::endl;

   int bestIndex = -1;
   int bestScore = 0;

   for (int i = 0; i < physicalDevices.size(); i++) {
    auto score = scoreDevice(physicalDevices[i]);
    if (bestIndex < 0 || score > bestScore) {
        bestIndex = i;
        bestScore = score;
    }
   }
  vk::PhysicalDeviceProperties properties = physicalDevices[bestIndex].getProperties();
  //vk::PhysicalDeviceFeatures features = physicalDevice.getFeatures();

  std::cout << "Found physical: " << properties.deviceName << std::endl;

  return physicalDevices[bestIndex];
}

int32_t RankedPhysicalDeviceStrategy::scoreDevice(const vk::PhysicalDevice& physicalDevice) {
  int32_t score = 0;
  //vk::PhysicalDeviceFeatures features;
  //vk::PhysicalDeviceProperties properties;
  std::vector<vk::ExtensionProperties> extensionProperties = physicalDevice.enumerateDeviceExtensionProperties();

  //vkGetPhysicalDeviceFeatures(device, &features);
  //vkGetPhysicalDeviceProperties(device, &properties);
  vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();
  vk::PhysicalDeviceFeatures features = physicalDevice.getFeatures();

  std::cout << "Scoring Physical Device: " << properties.deviceName << std::endl;

  if (!features.geometryShader) {
    std::cout << "\t-1000 (no geometry shader)" << std::endl;
    score -= 1000;
  }

  if (!features.tessellationShader) {
    std::cout << "\t-1000 (no tessellation shader)" << std::endl;
    score -= 1000;
  }

  if (!features.samplerAnisotropy) {
    std::cout << "\t-250 (no sampler anisotropy)" << std::endl;
    score -= 250;
  }

  if (!features.imageCubeArray) {
    std::cout << "\t-250 (no image cubemap)" << std::endl;
    score -= 250;
  }

  if (!features.fullDrawIndexUint32) {
     std::cout << "\t-500 (no full draw index uint32)" << std::endl;
    score -= 500;
  }

  if (features.multiDrawIndirect) {
     std::cout << "\t+250 (indirect multidraw)" << std::endl;

    score += 250;
  }

  if (features.textureCompressionETC2) {

     std::cout << "\t+250 (etc2 texture compression)" << std::endl;

    score += 250;
  }

  if (features.textureCompressionASTC_LDR) {

     std::cout << "\t+250 (ASTC_LDR texture compression)" << std::endl;

    score += 250;
  }

  auto descriptorSetsScore = 10 * properties.limits.maxBoundDescriptorSets;
  score += descriptorSetsScore;

  std::cout << "\t+" << descriptorSetsScore << " (descriptor set score)" << std::endl;

  auto descriptorComponentsScore = (5 * (properties.limits.maxPerStageDescriptorUniformBuffers +
            properties.limits.maxPerStageDescriptorSampledImages +
            properties.limits.maxPerStageDescriptorSamplers +
            properties.limits.maxPerStageDescriptorStorageImages));

  score += descriptorComponentsScore;


  std::cout << "\t+" << descriptorComponentsScore << " (descriptor components score)" << std::endl;


  if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {

     std::cout << "\t+800000000 (discrete GPU)" << std::endl;
    score += 800000000;
  }

  auto tex_sizes = {
      properties.limits.maxImageDimension1D,
      properties.limits.maxImageDimension2D,
      properties.limits.maxImageDimension3D,
  };

  for (auto& sz : tex_sizes) {
    auto texSizeScore = sz / 100;
    std::cout << "\t+" << texSizeScore << " (max tex size score)" << std::endl;
    score += texSizeScore;
  }


  std::cout << "Total: " << score << std::endl;
  return score;
}

}  // namespace engine


