#pragma once

#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <iostream>
#include <string>
#include <vector>

namespace engine {

class InstanceBuilder {
 public:
  InstanceBuilder(const std::string& applicationName, const std::string& engineName, uint32_t apiVersion);
  ~InstanceBuilder();

  InstanceBuilder(const InstanceBuilder&) = delete;
  InstanceBuilder& operator=(const InstanceBuilder&) = delete;

  InstanceBuilder& setLayers(std::vector<std::string> const& layers);
  InstanceBuilder& setExtensions(std::vector<std::string> const& extensions);
  vk::Instance build();

 private:
  std::string _applicationName;
  std::string _engineName;
  std::vector<std::string> _layers;
  std::vector<std::string> _extensions;
  uint32_t _apiVersion;

  std::vector<char const*> gatherLayers(std::vector<std::string> const& layers);
  std::vector<char const*> gatherLayers(
      std::vector<std::string> const& layers,
      std::vector<vk::LayerProperties> const& layerProperties);

  std::vector<char const*> gatherExtensions(std::vector<std::string> const& extensions);
  std::vector<char const*> gatherExtensions(
      std::vector<std::string> const& extensions,
      std::vector<vk::ExtensionProperties> const& extensionProperties);

#if defined( NDEBUG )
    vk::StructureChain<vk::InstanceCreateInfo>
#else
    vk::StructureChain<vk::InstanceCreateInfo, vk::DebugUtilsMessengerCreateInfoEXT>
#endif
      makeInstanceCreateInfoChain( vk::InstanceCreateFlagBits        instanceCreateFlagBits,
                                   vk::ApplicationInfo const &       applicationInfo,
                                   std::vector<char const *> const & layers,
                                   std::vector<char const *> const & extensions );

};

}  // namespace engine