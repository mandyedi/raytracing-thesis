#ifndef RT_IMAGE_H
#define RT_IMAGE_H

#include <string>
#include <vector>
#include "math/rt_vector.h"

// Rendered image: one RGB color per pixel, rows from top to bottom
class RTImage
{
public:
    RTImage( int width, int height );

    int getWidth() const;
    int getHeight() const;

    inline void setPixel( int row, int col, const RTVector &color )
    {
        Pixels[row * Width + col] = color;
    }

    inline const RTVector& getPixel( int row, int col ) const
    {
        return Pixels[row * Width + col];
    }

    // 3 bytes (R, G, B) per pixel; colors are clamped to [0, 1] and scaled to [0, 255]
    std::vector<unsigned char> toRGB8() const;

    bool savePNG( const std::string &fileName ) const;

private:
    int Width;
    int Height;
    std::vector<RTVector> Pixels;
};

#endif // RT_IMAGE_H
