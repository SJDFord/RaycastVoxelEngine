#include "image_generators.hpp"

#include <cstring>

namespace engine {
CheckerboardImageGenerator::CheckerboardImageGenerator( std::array<uint8_t, 3> const & rgb0, std::array<uint8_t, 3> const & rgb1 )
      : m_rgb0( rgb0 ), m_rgb1( rgb1 )
    {
    }

    void CheckerboardImageGenerator::operator()( void * data, vk::Extent2D & extent ) const
    {
      // Checkerboard of 16x16 pixel squares
      uint8_t * pImageMemory = static_cast<uint8_t *>( data );
      for ( uint32_t row = 0; row < extent.height; row++ )
      {
        for ( uint32_t col = 0; col < extent.width; col++ )
        {
          std::array<uint8_t, 3> const & rgb = ( ( ( row & 0x10 ) == 0 ) ^ ( ( col & 0x10 ) == 0 ) ) ? m_rgb1 : m_rgb0;
          pImageMemory[0]                    = rgb[0];
          pImageMemory[1]                    = rgb[1];
          pImageMemory[2]                    = rgb[2];
          pImageMemory[3]                    = 255;
          pImageMemory += 4;
        }
      }
    }

    MonochromeImageGenerator::MonochromeImageGenerator( std::array<unsigned char, 3> const & rgb ) : m_rgb( rgb ) {}

    void MonochromeImageGenerator::operator()( void * data, vk::Extent2D const & extent ) const
    {
      // fill in with the monochrome color
      unsigned char * pImageMemory = static_cast<unsigned char *>( data );
      for ( uint32_t row = 0; row < extent.height; row++ )
      {
        for ( uint32_t col = 0; col < extent.width; col++ )
        {
          pImageMemory[0] = m_rgb[0];
          pImageMemory[1] = m_rgb[1];
          pImageMemory[2] = m_rgb[2];
          pImageMemory[3] = 255;
          pImageMemory += 4;
        }
      }
    }

    PixelsImageGenerator::PixelsImageGenerator( vk::Extent2D const & extent, size_t channels, unsigned char const * pixels )
      : m_extent( extent ), m_channels( channels ), m_pixels( pixels )
    {
      assert( m_channels == 4 );
    }

    void PixelsImageGenerator::operator()( void * data, vk::Extent2D const & extent ) const
    {
      assert( extent == m_extent );
      memcpy( data, m_pixels, extent.width * extent.height * m_channels );
    }

}
