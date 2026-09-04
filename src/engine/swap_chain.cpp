#include "swap_chain.hpp"

#include <algorithm>

namespace engine {


    SwapChain::SwapChain( vk::PhysicalDevice const & physicalDevice,
                                  vk::Device const &         device,
                                  vk::SurfaceKHR const &     surface,
                                  vk::Extent2D const &       extent,
                                  vk::ImageUsageFlags        usage,
                                  uint32_t                   graphicsQueueFamilyIndex,
                                  uint32_t                   presentQueueFamilyIndex,
                                  uint32_t                   maxFramesInFlight,
                                  std::shared_ptr<SwapChain> previous): 
                                  _device{device}, 
                                  _physicalDevice{physicalDevice},
                                  _graphicsFamilyIndex{graphicsQueueFamilyIndex},
                                  _presentFamilyIndex{presentQueueFamilyIndex},
                                  _maxFramesInFlight{maxFramesInFlight}
    {
      vk::SurfaceFormatKHR surfaceFormat = engine::pickSurfaceFormat( physicalDevice.getSurfaceFormatsKHR( surface ) );
      _colorFormat = surfaceFormat.format;

      vk::SurfaceCapabilitiesKHR surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR( surface );
      if ( surfaceCapabilities.currentExtent.width == ( std::numeric_limits<uint32_t>::max )() )
      {
        // If the surface size is undefined, the size is set to the size of the images requested.
        _extent.width  = std::clamp( extent.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width );
        _extent.height = std::clamp( extent.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height );
      }
      else
      {
        // If the surface size is defined, the swap chain size must match
        _extent = surfaceCapabilities.currentExtent;
      }
      vk::SurfaceTransformFlagBitsKHR preTransform = ( surfaceCapabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity )
                                                     ? vk::SurfaceTransformFlagBitsKHR::eIdentity
                                                     : surfaceCapabilities.currentTransform;
      vk::CompositeAlphaFlagBitsKHR   compositeAlpha =
        ( surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePreMultiplied )    ? vk::CompositeAlphaFlagBitsKHR::ePreMultiplied
          : ( surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePostMultiplied ) ? vk::CompositeAlphaFlagBitsKHR::ePostMultiplied
          : ( surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::eInherit )        ? vk::CompositeAlphaFlagBitsKHR::eInherit
                                                                                                             : vk::CompositeAlphaFlagBitsKHR::eOpaque;
      vk::PresentModeKHR         presentMode = engine::pickPresentMode( physicalDevice.getSurfacePresentModesKHR( surface ) );
      vk::SwapchainCreateInfoKHR swapChainCreateInfo(
        {},
        surface,
        engine::clampSurfaceImageCount( 3u, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount ),
        _colorFormat,
        surfaceFormat.colorSpace,
        _extent,
        1,
        usage,
        vk::SharingMode::eExclusive,
        {},
        preTransform,
        compositeAlpha,
        presentMode,
        true,
        previous == nullptr ? VK_NULL_HANDLE : previous->getSwapChain());
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

      createDepthResources();
      createSyncObjects();
    }

    SwapChain::~SwapChain() {
      /* TODO: Determine why device is invalid at this point
      for (auto imageView : _imageViews) {
        _device.destroyImageView(imageView);
      }
      _imageViews.clear();

      if (_swapChain != nullptr) {
        _device.destroySwapchainKHR(_swapChain);
        _swapChain = nullptr;
      }

      for (int i = 0; i < _depthImages.size(); i++) {
        _device.destroyImageView(_depthImageViews[i]);
        _device.destroyImage(_depthImages[i]);
        _device.freeMemory(_depthImageMemorys[i]);
      }

      // cleanup synchronization objects
      for (size_t i = 0; i < _maxFramesInFlight; i++) {
        _device.destroySemaphore(_renderFinishedSemaphores[i]);
        _device.destroySemaphore(_imageAvailableSemaphores[i]);
        _device.destroyFence(_inFlightFences[i]);
      }
      */
    }


    const vk::SwapchainKHR& SwapChain::getSwapChain() const {
        return _swapChain;
    }
    const std::vector<vk::ImageView>& SwapChain::getImageViews() const {
        return _imageViews;
    }


    const std::vector<vk::Image>& SwapChain::getImages() const {
      return _images;
    }

    const vk::Format SwapChain::getFormat() const {
      return _colorFormat;
    }

    const vk::Format SwapChain::getDepthFormat() const {
      return _depthFormat;
    }


    vk::ResultValue<uint32_t> SwapChain::acquireNextImage() {
      _device.waitForFences(_inFlightFences[_currentFrame], true, 
          std::numeric_limits<uint64_t>::max());

      vk::AcquireNextImageInfoKHR acquireNextImageInfo =
          vk::AcquireNextImageInfoKHR()
              .setSwapchain(_swapChain)
              .setTimeout(std::numeric_limits<uint64_t>::max())
              .setSemaphore(_imageAvailableSemaphores[_currentFrame])
              .setFence(VK_NULL_HANDLE)
              .setDeviceMask(1);
 
      vk::ResultValue<uint32_t> result = _device.acquireNextImage2KHR(acquireNextImageInfo);
            
      return result;
    }

     vk::Result SwapChain::submitCommandBuffer(
        vk::CommandBuffer buffer, uint32_t imageIndex) {
      
      if (_imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
        _device.waitForFences(
            _imagesInFlight[imageIndex],
            true,
            std::numeric_limits<uint64_t>::max());
      }
      _imagesInFlight[imageIndex] = _inFlightFences[_currentFrame];
      //imagesInFlight.insert(imagesInFlight.begin() + imageIndex, inFlightFences[currentFrame]);
      vk::Semaphore signalSemaphore = _renderFinishedSemaphores[_currentFrame];
      vk::PipelineStageFlags waitFlags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
      vk::Semaphore &waitSemaphore = _imageAvailableSemaphores[_currentFrame];
      vk::SubmitInfo submitInfo = vk::SubmitInfo()
        .setWaitSemaphoreCount(1)
        .setWaitSemaphores(waitSemaphore)
        .setWaitDstStageMask(waitFlags)
        .setCommandBufferCount(1)
        .setCommandBuffers(buffer)
        .setSignalSemaphoreCount(1)
        .setSignalSemaphores(signalSemaphore);

      _device.resetFences(_inFlightFences[_currentFrame]);
      // TODO: Get graphics queue
      _device.getQueue(_graphicsFamilyIndex, 0).submit(submitInfo, _inFlightFences[_currentFrame]);
      vk::PresentInfoKHR presentInfo =
          vk::PresentInfoKHR()
          .setWaitSemaphoreCount(1)
          .setWaitSemaphores(signalSemaphore)
          .setSwapchainCount(1)
          .setSwapchains(_swapChain)
          .setImageIndices(imageIndex);

      // TODO: Get present queue
      vk::Result result = _device.getQueue(_presentFamilyIndex, 0).presentKHR(presentInfo);

      _currentFrame = (_currentFrame + 1) % _maxFramesInFlight;

      return result;
    }

    void SwapChain::createDepthResources() {
      vk::Format depthFormat = findDepthFormat();
      _depthFormat = depthFormat;

      _depthImages.resize(imageCount());
      _depthImageMemorys.resize(imageCount());
      _depthImageViews.resize(imageCount());

      for (int i = 0; i < _depthImages.size(); i++) {
        vk::Extent3D extent = vk::Extent3D()
                                  .setWidth(_extent.width)
                                  .setHeight(_extent.height)
                                  .setDepth(1);
        vk::ImageCreateInfo imageInfo = vk::ImageCreateInfo()
                                            .setImageType(vk::ImageType::e2D)
                                            .setExtent(extent)
                                            .setMipLevels(1)
                                            .setArrayLayers(1)
                                            .setFormat(depthFormat)
                                            .setTiling(vk::ImageTiling::eOptimal)
                                            .setInitialLayout(vk::ImageLayout::eUndefined)
                                            .setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment)
                                            .setSamples(vk::SampleCountFlagBits::e1)
                                            .setSharingMode(vk::SharingMode::eExclusive);
        // TODO: .setFlags(vk::ImageCreateFlagBits::) imageInfo.flags = 0;
        
        auto result = createImageWithInfo(imageInfo, vk::MemoryPropertyFlagBits::eDeviceLocal);       
        _depthImages[i] = result.image;
        _depthImageMemorys[i] = result.imageMemory;

        vk::ImageSubresourceRange subresourceRange = vk::ImageSubresourceRange()
                                                        .setAspectMask(vk::ImageAspectFlagBits::eDepth)
                                                        .setBaseMipLevel(0)
                                                        .setLevelCount(1)
                                                        .setBaseArrayLayer(0)
                                                        .setLayerCount(1);

        vk::ImageViewCreateInfo viewInfo = vk::ImageViewCreateInfo()
            .setImage(_depthImages[i])
            .setViewType(vk::ImageViewType::e2D)
            .setFormat(depthFormat)
            .setSubresourceRange(subresourceRange);
        _depthImageViews[i] = _device.createImageView(viewInfo);
      }
    }

    void SwapChain::createSyncObjects() {
      _imageAvailableSemaphores.resize(_maxFramesInFlight, VK_NULL_HANDLE);
      _renderFinishedSemaphores.resize(_maxFramesInFlight, VK_NULL_HANDLE);
      _inFlightFences.resize(_maxFramesInFlight, VK_NULL_HANDLE);
      _imagesInFlight.resize(imageCount(), VK_NULL_HANDLE);

      vk::SemaphoreCreateInfo semaphoreInfo = vk::SemaphoreCreateInfo();

      vk::FenceCreateInfo fenceInfo = vk::FenceCreateInfo()
          .setFlags(vk::FenceCreateFlagBits::eSignaled);

      for (size_t i = 0; i < _maxFramesInFlight; i++) {
        _imageAvailableSemaphores[i] = _device.createSemaphore(semaphoreInfo);
        _renderFinishedSemaphores[i] = _device.createSemaphore(semaphoreInfo);
        _inFlightFences[i] = _device.createFence(fenceInfo);
      }
    }

    vk::Format SwapChain::findDepthFormat() {
      return findSupportedFormat(
          {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
          vk::ImageTiling::eOptimal,
          vk::FormatFeatureFlagBits::eDepthStencilAttachment
      );
    }

    vk::Format SwapChain::findSupportedFormat(
        const std::vector<vk::Format> &candidates,
        vk::ImageTiling tiling,
        vk::FormatFeatureFlags features) {
      for (vk::Format format : candidates) {
        
          
        vk::FormatProperties props = _physicalDevice.getFormatProperties(format);
        if (tiling ==  vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
          return format;
        } else if (
            tiling == vk::ImageTiling::eOptimal &&
            (props.optimalTilingFeatures & features) == features) {
          return format;
        }
      }
      throw std::runtime_error("failed to find supported format!");
    }
}
