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


std::vector<vk::DescriptorImageInfo> ImageRegistry::getDescriptors() {
  std::vector<vk::DescriptorImageInfo> imageInfoDescriptors;
  imageInfoDescriptors.resize(_maxImageCount);
  for (uint32_t i = 0; i < _images.size(); ++i) {
    imageInfoDescriptors[i].sampler = nullptr;
	  imageInfoDescriptors[i].imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    imageInfoDescriptors[i].imageView = _images[i]->GetImageView();
  }
  return imageInfoDescriptors;
}

}