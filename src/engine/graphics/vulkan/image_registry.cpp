#include "image_registry.hpp"

namespace engine {


ImageRegistry::ImageRegistry(size_t maxImageCount): _maxImageCount{maxImageCount} {

}

ImageRegistry::~ImageRegistry() {

}


uint32_t ImageRegistry::addImage(std::shared_ptr<Image> image) {
    assert(_images.size() < _maxImageCount && "Max image count exceeded");
    
    _images.push_back(image);
    return _images.size() - 1;
}


std::vector<std::shared_ptr<Image>>& ImageRegistry::getImages() {
    return _images;
}

}