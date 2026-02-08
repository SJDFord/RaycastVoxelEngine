#include "swap_chain.hpp"

#include <algorithm>

namespace engine {


    SwapChain::SwapChain( vk::PhysicalDevice const & physicalDevice,
                                  vk::Device const &         device,
                                  vk::SurfaceKHR const &     surface,
                                  vk::Extent2D const &       extent,
                                  vk::ImageUsageFlags        usage,
                                  vk::SwapchainKHR const &   oldSwapChain,
                                  uint32_t                   graphicsQueueFamilyIndex,
                                  uint32_t                   presentQueueFamilyIndex )
    {
      vk::SurfaceFormatKHR surfaceFormat = VulkanUtils::pickSurfaceFormat( physicalDevice.getSurfaceFormatsKHR( surface ) );
      _colorFormat = surfaceFormat.format;

      vk::SurfaceCapabilitiesKHR surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR( surface );
      vk::Extent2D               swapchainExtent;
      if ( surfaceCapabilities.currentExtent.width == ( std::numeric_limits<uint32_t>::max )() )
      {
        // If the surface size is undefined, the size is set to the size of the images requested.
        swapchainExtent.width  = std::clamp( extent.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width );
        swapchainExtent.height = std::clamp( extent.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height );
      }
      else
      {
        // If the surface size is defined, the swap chain size must match
        swapchainExtent = surfaceCapabilities.currentExtent;
      }
      vk::SurfaceTransformFlagBitsKHR preTransform = ( surfaceCapabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity )
                                                     ? vk::SurfaceTransformFlagBitsKHR::eIdentity
                                                     : surfaceCapabilities.currentTransform;
      vk::CompositeAlphaFlagBitsKHR   compositeAlpha =
        ( surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePreMultiplied )    ? vk::CompositeAlphaFlagBitsKHR::ePreMultiplied
          : ( surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePostMultiplied ) ? vk::CompositeAlphaFlagBitsKHR::ePostMultiplied
          : ( surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::eInherit )        ? vk::CompositeAlphaFlagBitsKHR::eInherit
                                                                                                             : vk::CompositeAlphaFlagBitsKHR::eOpaque;
      vk::PresentModeKHR         presentMode = VulkanUtils::pickPresentMode( physicalDevice.getSurfacePresentModesKHR( surface ) );
      vk::SwapchainCreateInfoKHR swapChainCreateInfo(
        {},
        surface,
        VulkanUtils::clampSurfaceImageCount( 3u, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount ),
        _colorFormat,
        surfaceFormat.colorSpace,
        swapchainExtent,
        1,
        usage,
        vk::SharingMode::eExclusive,
        {},
        preTransform,
        compositeAlpha,
        presentMode,
        true,
        oldSwapChain );
      if ( graphicsQueueFamilyIndex != presentQueueFamilyIndex )
      {
        uint32_t queueFamilyIndices[2] = { graphicsQueueFamilyIndex, presentQueueFamilyIndex };
        // If the graphics and present queues are from different queue families, we either have to explicitly transfer
        // ownership of images between the queues, or we have to create the swapchain with imageSharingMode as
        // vk::SharingMode::eConcurrent
        swapChainCreateInfo.imageSharingMode      = vk::SharingMode::eConcurrent;
        swapChainCreateInfo.queueFamilyIndexCount = 2;
        swapChainCreateInfo.pQueueFamilyIndices   = queueFamilyIndices;
      }
      _swapChain = device.createSwapchainKHR( swapChainCreateInfo );

      _images = device.getSwapchainImagesKHR( _swapChain );

      _imageViews.reserve( _images.size() );
      vk::ImageViewCreateInfo imageViewCreateInfo( {}, {}, vk::ImageViewType::e2D, _colorFormat, {}, { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 } );
      for ( auto image : _images )
      {
        imageViewCreateInfo.image = image;
        _imageViews.push_back( device.createImageView( imageViewCreateInfo ) );
      }
    }

SwapChain::~SwapChain() {
}


    const vk::SwapchainKHR& SwapChain::getSwapChain() const {
        return _swapChain;
    }
    const std::vector<vk::ImageView>& SwapChain::getImageViews() const {
        return _imageViews;
    }

}
