#include "image_view_create_info_builder.hpp"

namespace engine {

ImageViewCreateInfoBuilder::ImageViewCreateInfoBuilder() {

}

ImageViewCreateInfoBuilder::~ImageViewCreateInfoBuilder() {
    
}

ImageViewCreateInfoBuilder& ImageViewCreateInfoBuilder::setFlags(vk::ImageAspectFlags flags) {
    _flags = flags;
    return *this;
}

ImageViewCreateInfoBuilder& ImageViewCreateInfoBuilder::setImage(vk::Image image) {
    _image = image;
    return *this;
}

ImageViewCreateInfoBuilder& ImageViewCreateInfoBuilder::setFormat(vk::Format format) {
    _format = format;
    return *this;
}

vk::ImageViewCreateInfo ImageViewCreateInfoBuilder::build() {
    vk::ImageSubresourceRange subresourceRange = vk::ImageSubresourceRange(_flags)
        .setBaseMipLevel(0)
        .setLevelCount(1)
        .setBaseArrayLayer(0)
        .setLayerCount(1);

    vk::ImageViewCreateInfo viewInfo = vk::ImageViewCreateInfo()
        .setImage(_image)
        .setViewType(vk::ImageViewType::e2D)
        .setFormat(_format)
        .setSubresourceRange(subresourceRange);

    return viewInfo;
}

}  // namespace engine
