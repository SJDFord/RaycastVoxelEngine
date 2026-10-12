#pragma once

#include <array>
#include <cstdint>

#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_raii.hpp"

namespace engine {

class CheckerboardImageGenerator
{
    public:
      CheckerboardImageGenerator( std::array<uint8_t, 3> const & rgb0 = { { 0, 0, 0 } }, std::array<uint8_t, 3> const & rgb1 = { { 255, 255, 255 } } );

      void operator()( void * data, vk::Extent2D & extent ) const;

    private:
      std::array<uint8_t, 3> const & m_rgb0;
      std::array<uint8_t, 3> const & m_rgb1;
};

class MonochromeImageGenerator
{
    public:
      MonochromeImageGenerator( std::array<unsigned char, 3> const & rgb );

      void operator()( void * data, vk::Extent2D const & extent ) const;

    private:
      std::array<unsigned char, 3> const & m_rgb;
};

class PixelsImageGenerator
{
    public:
      PixelsImageGenerator( vk::Extent2D const & extent, size_t channels, unsigned char const * pixels );

      void operator()( void * data, vk::Extent2D const & extent ) const;

    private:
      vk::Extent2D          m_extent;
      size_t                m_channels;
      unsigned char const * m_pixels;
};

}
