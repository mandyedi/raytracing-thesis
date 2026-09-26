#ifndef RT_RENDERER_H
#define RT_RENDERER_H

#include "rt_image.h"

class RTScene;
class RTCamera;

struct RTRenderSettings
{
    int  Width           = 800;
    int  Height          = 600;
    int  NumberOfThreads = 1;
    int  MaxTraceDepth   = 3;
    bool UseSIMD         = true;
    int  TileWidth       = 32;
    int  TileHeight      = 32;
};

// Renders a scene on several threads. The image is split into tiles, and each thread takes tiles until none are left.
class RTRenderer
{
public:
    explicit RTRenderer( const RTRenderSettings &settings );

    // Blocks until the image is done. Rays come from a copy of the camera, sized to the image.
    RTImage render( RTScene *scene, const RTCamera &camera );

    // Seconds the last render took
    double getRenderTime();

private:
    RTRenderSettings Settings;
    double           RenderTime;
};

#endif // RT_RENDERER_H
