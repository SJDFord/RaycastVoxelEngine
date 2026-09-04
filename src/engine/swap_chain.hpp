#pragma once

#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_raii.hpp"
#include "utils.hpp"

namespace engine {

struct ImageWithMemory {
  vk::Image image;
  vk::DeviceMemory imageMemory;
};

class SwapChain {
public:
    SwapChain( vk::PhysicalDevice const & physicalDevice,
                     vk::Device const &         device,
                     vk::SurfaceKHR const &     surface,
                     vk::Extent2D const &       extent,
                     vk::ImageUsageFlags        usage,
                     uint32_t                   graphicsFamilyIndex,
                     uint32_t                   presentFamilyIndex,
                     uint32_t                   maxFramesInFlight,
                     std::shared_ptr<SwapChain> previous = nullptr);
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
    const std::vector<vk::Image>& getImages() const;
    const vk::Format getFormat() const;
    const vk::Format getDepthFormat() const;
    const uint32_t getMaxFramesInFlight() const { return _maxFramesInFlight; };

    size_t imageCount() { return _images.size(); }
    vk::ResultValue<uint32_t> acquireNextImage();
    vk::Result submitCommandBuffer(vk::CommandBuffer buffer, uint32_t imageIndex);

    bool compareSwapFormats(const SwapChain &swapChain) const {
        return swapChain.getDepthFormat() == getDepthFormat() &&
           swapChain.getDepthFormat() == getDepthFormat();
    }

private:
    const vk::Device& _device;
    const vk::PhysicalDevice& _physicalDevice;
    const uint32_t _graphicsFamilyIndex;
    const uint32_t _presentFamilyIndex;
    const uint32_t _maxFramesInFlight;

    vk::Format                 _colorFormat;
    vk::Format                 _depthFormat;
    vk::Extent2D               _extent;
    vk::SwapchainKHR           _swapChain;

    std::vector<vk::Image> _depthImages;
    std::vector<vk::DeviceMemory> _depthImageMemorys;
    std::vector<vk::ImageView> _depthImageViews;
    std::vector<vk::Image>     _images;
    std::vector<vk::ImageView> _imageViews;

    std::vector<vk::Semaphore> _imageAvailableSemaphores;
    std::vector<vk::Semaphore> _renderFinishedSemaphores;
    std::vector<vk::Fence> _inFlightFences;
    std::vector<vk::Fence> _imagesInFlight;
    size_t _currentFrame = 0;

    void createDepthResources();
    void createSyncObjects();
      vk::Format findDepthFormat();
      vk::Format findSupportedFormat(const std::vector<vk::Format> &candidates,
        vk::ImageTiling tiling,
        vk::FormatFeatureFlags features);

    
    ImageWithMemory createImageWithInfo(
        const vk::ImageCreateInfo &imageInfo,
        vk::MemoryPropertyFlags properties) {
        ImageWithMemory result = {};
        result.image =_device.createImage(imageInfo);
        
        vk::PhysicalDeviceMemoryProperties memoryProperties = _physicalDevice.getMemoryProperties();
        vk::MemoryRequirements memRequirements = _device.getImageMemoryRequirements(result.image);

        vk::MemoryAllocateInfo allocInfo =
            vk::MemoryAllocateInfo()
                .setAllocationSize(memRequirements.size)
                .setMemoryTypeIndex(findMemoryType(memoryProperties, memRequirements.memoryTypeBits, properties));
        result.imageMemory = _device.allocateMemory(allocInfo);
        _device.bindImageMemory(result.image, result.imageMemory, 0);
        return result;
    }
    
};

}

