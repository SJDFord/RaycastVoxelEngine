#pragma once

#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_raii.hpp"
#include "../util/vulkan_utils.hpp"

namespace engine {

class SwapChain {
public:
    SwapChain( vk::PhysicalDevice const & physicalDevice,
                     vk::Device const &         device,
                     vk::SurfaceKHR const &     surface,
                     vk::Extent2D const &       extent,
                     vk::ImageUsageFlags        usage,
                     vk::SwapchainKHR const &   oldSwapChain,
                     uint32_t                   graphicsFamilyIndex,
                     uint32_t                   presentFamilyIndex);
    ~SwapChain();

    SwapChain(const SwapChain&) = delete;
    SwapChain& operator=(const SwapChain&) = delete;
    void clear( vk::Device const & device )
    {
        for ( auto & imageView : _imageViews )
        {
          device.destroyImageView( imageView );
        }
        _imageViews.clear();
        _images.clear();
        device.destroySwapchainKHR( _swapChain );
    }
    
    const vk::SwapchainKHR& getSwapChain() const;
    const std::vector<vk::ImageView>& getImageViews() const;

private:
    vk::Format                 _colorFormat;
    vk::SwapchainKHR           _swapChain;
    std::vector<vk::Image>     _images;
    std::vector<vk::ImageView> _imageViews;
};

}

