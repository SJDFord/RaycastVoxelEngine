#pragma once

#include <vector>
#include <memory>

#include "./image.hpp"

namespace engine {
class ImageRegistry {
public:

    ImageRegistry(size_t maxImageCount);
    ~ImageRegistry();

    ImageRegistry(const ImageRegistry &) = delete;
    ImageRegistry &operator=(const ImageRegistry &) = delete;

    uint32_t addImage(std::shared_ptr<Image> image);

    std::vector<vk::DescriptorImageInfo> getDescriptors();
private:
    size_t _maxImageCount;
    std::vector<std::shared_ptr<Image>> _images; 
};
}