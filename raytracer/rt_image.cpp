#include <algorithm>
#include "rt_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "3rd_party/stb_image_write.h"

static unsigned char toByte( float value )
{
    return static_cast<unsigned char>( std::max( 0.0f, std::min( 1.0f, value ) ) * 255 );
}

// stbi_write_png_to_func callback: appends a chunk of the PNG file to a byte vector
static void appendBytes( void *context, void *data, int size )
{
    std::vector<unsigned char> *bytes = static_cast<std::vector<unsigned char>*>( context );
    const unsigned char *chunk = static_cast<const unsigned char*>( data );
    bytes->insert( bytes->end(), chunk, chunk + size );
}

RTImage::RTImage( int width, int height )
    : Width( width )
    , Height( height )
    , Pixels( static_cast<size_t>( width ) * height, RTVector( 0.0f, 0.0f, 0.0f ) )
{
}

int RTImage::getWidth() const
{
    return Width;
}

int RTImage::getHeight() const
{
    return Height;
}

std::vector<unsigned char> RTImage::toRGB8() const
{
    std::vector<unsigned char> rgb( Pixels.size() * 3 );
    for ( size_t i = 0; i < Pixels.size(); i++ )
    {
        rgb[i * 3]     = toByte( Pixels[i].x() );
        rgb[i * 3 + 1] = toByte( Pixels[i].y() );
        rgb[i * 3 + 2] = toByte( Pixels[i].z() );
    }
    return rgb;
}

bool RTImage::savePNG( const std::string &fileName ) const
{
    std::vector<unsigned char> rgb = toRGB8();
    return stbi_write_png( fileName.c_str(), Width, Height, 3, rgb.data(), Width * 3 ) != 0;
}

std::vector<unsigned char> RTImage::toPNG() const
{
    std::vector<unsigned char> rgb = toRGB8();
    std::vector<unsigned char> png;
    if ( stbi_write_png_to_func( appendBytes, &png, Width, Height, 3, rgb.data(), Width * 3 ) == 0 )
    {
        png.clear();
    }
    return png;
}
