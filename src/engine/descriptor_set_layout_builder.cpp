#include "descriptor_set_layout_builder.hpp"

namespace engine {

DescriptorSetLayoutBuilder::DescriptorSetLayoutBuilder(vk::Device const& device) {
  _device = device;
}

DescriptorSetLayoutBuilder::~DescriptorSetLayoutBuilder() {}

DescriptorSetLayoutBuilder& DescriptorSetLayoutBuilder::addBinding(
    vk::DescriptorType descriptorType, uint32_t descriptorCount, vk::ShaderStageFlags stageFlags) {
  _bindingData.push_back({descriptorType, descriptorCount, stageFlags});
  return *this;
}
DescriptorSetLayoutBuilder& DescriptorSetLayoutBuilder::setFlags(
    vk::DescriptorSetLayoutCreateFlags flags) {
  _flags = flags;
  return *this;
}

vk::DescriptorSetLayout DescriptorSetLayoutBuilder::build() {
  std::vector<vk::DescriptorSetLayoutBinding> bindings(_bindingData.size());
  for (size_t i = 0; i < _bindingData.size(); i++) {
    bindings[i] = vk::DescriptorSetLayoutBinding(
        static_cast<uint32_t>(i),
        std::get<0>(_bindingData[i]),
        std::get<1>(_bindingData[i]),
        std::get<2>(_bindingData[i]));
  }
  return _device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo(_flags, bindings));
}

/*

    vk::DescriptorSetLayout createDescriptorSetLayout( vk::Device const & device,
                                                       std::vector<std::tuple<vk::DescriptorType,
   uint32_t, vk::ShaderStageFlags>> const & bindingData, vk::DescriptorSetLayoutCreateFlags flags )
    {
      std::vector<vk::DescriptorSetLayoutBinding> bindings( bindingData.size() );
      for ( size_t i = 0; i < bindingData.size(); i++ )
      {
        bindings[i] = vk::DescriptorSetLayoutBinding(
          checked_cast<uint32_t>( i ), std::get<0>( bindingData[i] ), std::get<1>( bindingData[i] ),
   std::get<2>( bindingData[i] ) );
      }
      return device.createDescriptorSetLayout( vk::DescriptorSetLayoutCreateInfo( flags, bindings )
   );
    }
*/

}  // namespace engine

