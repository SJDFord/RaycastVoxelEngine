#include "image_create_info_builder.hpp"

namespace engine {

ImageCreateInfoBuilder::ImageCreateInfoBuilder() {

}

ImageCreateInfoBuilder::~ImageCreateInfoBuilder() {
    
}

ImageCreateInfoBuilder& ImageCreateInfoBuilder::setExtent(vk::Extent2D extent) {
    _extent = extent;
    return *this;
}

ImageCreateInfoBuilder& ImageCreateInfoBuilder::setFormat(vk::Format format) {
    _format = format;
    return *this;
}

ImageCreateInfoBuilder& ImageCreateInfoBuilder::setUsageFlags(vk::ImageUsageFlags usageFlags) {
    _usageFlags = usageFlags;
    return *this;
}

ImageCreateInfoBuilder& ImageCreateInfoBuilder::setSharingMode(vk::SharingMode sharingMode) {
    _sharingMode = sharingMode;
    return *this;
}

vk::ImageCreateInfo ImageCreateInfoBuilder::build() {
    vk::ImageCreateInfo imageInfo = vk::ImageCreateInfo()
        .setImageType(vk::ImageType::e2D)
        .setExtent(vk::Extent3D(_extent, 1))
        .setMipLevels(1)
        .setArrayLayers(1)
        .setFormat(_format)
        .setTiling(vk::ImageTiling::eOptimal)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setUsage(_usageFlags)
        .setSamples(vk::SampleCountFlagBits::e1)
        .setSharingMode(_sharingMode)
        .setFlags({});

    return imageInfo;
}
}