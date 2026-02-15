#include "instance_builder.hpp"


static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugUtilsMessengerCallback(
    vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    vk::DebugUtilsMessageTypeFlagsEXT messageTypes,
    const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* /*pUserData*/) {
#if !defined(NDEBUG)
  switch (static_cast<uint32_t>(pCallbackData->messageIdNumber)) {
    case 0:
      // Validation Warning: Override layer has override paths set to C:/VulkanSDK/<version>/Bin
      return vk::False;
    case 0x822806fa:
      // Validation Warning: vkCreateInstance(): to enable extension VK_EXT_debug_utils, but this
      // extension is intended to support use by applications when debugging and it is strongly
      // recommended that it be otherwise avoided.
      return vk::False;
    case 0xe8d1a9fe:
      // Validation Performance Warning: Using debug builds of the validation layers *will*
      // adversely affect performance.
      return vk::False;
  }
#endif

  std::cerr << vk::to_string(messageSeverity) << ": " << vk::to_string(messageTypes) << ":\n";
  std::cerr << std::string("\t") << "messageIDName   = <" << pCallbackData->pMessageIdName << ">\n";
  std::cerr << std::string("\t") << "messageIdNumber = " << pCallbackData->messageIdNumber << "\n";
  std::cerr << std::string("\t") << "message         = <" << pCallbackData->pMessage << ">\n";
  if (0 < pCallbackData->queueLabelCount) {
    std::cerr << std::string("\t") << "Queue Labels:\n";
    for (uint32_t i = 0; i < pCallbackData->queueLabelCount; i++) {
      std::cerr << std::string("\t\t") << "labelName = <"
                << pCallbackData->pQueueLabels[i].pLabelName << ">\n";
    }
  }
  if (0 < pCallbackData->cmdBufLabelCount) {
    std::cerr << std::string("\t") << "CommandBuffer Labels:\n";
    for (uint32_t i = 0; i < pCallbackData->cmdBufLabelCount; i++) {
      std::cerr << std::string("\t\t") << "labelName = <"
                << pCallbackData->pCmdBufLabels[i].pLabelName << ">\n";
    }
  }
  if (0 < pCallbackData->objectCount) {
    std::cerr << std::string("\t") << "Objects:\n";
    for (uint32_t i = 0; i < pCallbackData->objectCount; i++) {
      std::cerr << std::string("\t\t") << "Object " << i << "\n";
      std::cerr << std::string("\t\t\t")
                << "objectType   = " << vk::to_string(pCallbackData->pObjects[i].objectType)
                << "\n";
      std::cerr << std::string("\t\t\t")
                << "objectHandle = " << pCallbackData->pObjects[i].objectHandle << "\n";
      if (pCallbackData->pObjects[i].pObjectName) {
        std::cerr << std::string("\t\t\t") << "objectName   = <"
                  << pCallbackData->pObjects[i].pObjectName << ">\n";
      }
    }
  }
  return vk::False;
}

namespace engine {

InstanceBuilder::InstanceBuilder() {
    _apiVersion = VK_API_VERSION_1_0;
}

InstanceBuilder::~InstanceBuilder() {}

InstanceBuilder& InstanceBuilder::setApplicationName(const std::string& applicationName) {
  _applicationName = applicationName;
  return *this;
}

InstanceBuilder& InstanceBuilder::setEngineName(const std::string& engineName) {
  _engineName = engineName;
  return *this;
}

InstanceBuilder& InstanceBuilder::setLayers(std::vector<std::string> const& layers) {
  _layers = layers;
  return *this;
}
InstanceBuilder& InstanceBuilder::setExtensions(std::vector<std::string> const& extensions) {
  _extensions = extensions;
  return *this;
}

InstanceBuilder& InstanceBuilder::setApiVersion(uint32_t apiVersion) {
  _apiVersion = apiVersion;
  return *this;
}

vk::Instance InstanceBuilder::build() {
#if (VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1)
  VULKAN_HPP_DEFAULT_DISPATCHER.init();
#endif

  vk::ApplicationInfo applicationInfo(_applicationName.c_str(), 1, _engineName.c_str(), 1, _apiVersion);
  std::vector<char const*> enabledLayers = gatherLayers(
      _layers
#if !defined(NDEBUG)
      ,
      vk::enumerateInstanceLayerProperties()
#endif
  );
  std::vector<char const*> enabledExtensions = gatherExtensions(
      _extensions
#if !defined(NDEBUG)
      ,
      vk::enumerateInstanceExtensionProperties()
#endif
  );

  vk::Instance instance = vk::createInstance(
      makeInstanceCreateInfoChain({}, applicationInfo, enabledLayers, enabledExtensions)
          .get<vk::InstanceCreateInfo>());

#if (VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1)
  // initialize function pointers for instance
  VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);
#endif

  return instance;
}

std::vector<char const*> InstanceBuilder::gatherLayers(std::vector<std::string> const& layers) {
  std::vector<char const*> enabledLayers;
  enabledLayers.reserve(layers.size());
  for (auto const& layer : layers) {
    enabledLayers.push_back(layer.data());
  }

  return enabledLayers;
}

std::vector<char const*> InstanceBuilder::gatherLayers(
    std::vector<std::string> const& layers,
    std::vector<vk::LayerProperties> const& layerProperties) {
  std::vector<char const*> enabledLayers;
  enabledLayers.reserve(layers.size());
  for (auto const& layer : layers) {
    assert(
        std::any_of(
            layerProperties.begin(),
            layerProperties.end(),
            [layer](vk::LayerProperties const& lp) { return layer == lp.layerName; }));
    enabledLayers.push_back(layer.data());
  }

  // Enable standard validation layer to find as much errors as possible!
  if (std::none_of(
          layers.begin(),
          layers.end(),
          [](std::string const& layer) { return layer == "VK_LAYER_KHRONOS_validation"; }) &&
      std::any_of(
          layerProperties.begin(),
          layerProperties.end(),
          [](vk::LayerProperties const& lp) {
            return (strcmp("VK_LAYER_KHRONOS_validation", lp.layerName) == 0);
          })) {
    enabledLayers.push_back("VK_LAYER_KHRONOS_validation");
  }

  return enabledLayers;
}

std::vector<char const*> InstanceBuilder::gatherExtensions(
    std::vector<std::string> const& extensions) {
  std::vector<char const*> enabledExtensions;
  enabledExtensions.reserve(extensions.size());
  for (auto const& ext : extensions) {
    enabledExtensions.push_back(ext.data());
  }
  return enabledExtensions;
}
std::vector<char const*> InstanceBuilder::gatherExtensions(
    std::vector<std::string> const& extensions,
    std::vector<vk::ExtensionProperties> const& extensionProperties) {
  std::vector<char const*> enabledExtensions;
  enabledExtensions.reserve(extensions.size());
  for (auto const& ext : extensions) {
    assert(
        std::any_of(
            extensionProperties.begin(),
            extensionProperties.end(),
            [ext](vk::ExtensionProperties const& ep) { return ext == ep.extensionName; }));
    enabledExtensions.push_back(ext.data());
  }
  if (std::none_of(
          extensions.begin(),
          extensions.end(),
          [](std::string const& extension) {
            return extension == VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
          }) &&
      std::any_of(
          extensionProperties.begin(),
          extensionProperties.end(),
          [](vk::ExtensionProperties const& ep) {
            return (strcmp(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, ep.extensionName) == 0);
          })) {
    enabledExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  return enabledExtensions;
}

#if defined(NDEBUG)
vk::StructureChain<vk::InstanceCreateInfo>
#else
vk::StructureChain<vk::InstanceCreateInfo, vk::DebugUtilsMessengerCreateInfoEXT>
#endif
InstanceBuilder::makeInstanceCreateInfoChain(
    vk::InstanceCreateFlagBits instanceCreateFlagBits,
    vk::ApplicationInfo const& applicationInfo,
    std::vector<char const*> const& layers,
    std::vector<char const*> const& extensions) {
#if defined(NDEBUG)
  // in non-debug mode just use the InstanceCreateInfo for instance creation
  vk::StructureChain<vk::InstanceCreateInfo> instanceCreateInfo(
      {instanceCreateFlagBits, &applicationInfo, layers, extensions});
#else
  // in debug mode, addionally use the debugUtilsMessengerCallback in instance creation!
  vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(
      vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
      vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
  vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(
      vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
      vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
      vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
  vk::StructureChain<vk::InstanceCreateInfo, vk::DebugUtilsMessengerCreateInfoEXT>
      instanceCreateInfo(
          {instanceCreateFlagBits, &applicationInfo, layers, extensions},
          {{}, severityFlags, messageTypeFlags, &debugUtilsMessengerCallback});
#endif
  return instanceCreateInfo;
}

} // namespace engine

