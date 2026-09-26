#include <algorithm>
#include <chrono>
#include <thread>
#include <vector>

#include "rt_renderer.h"
#include "rt_image_parts.h"
#include "rt_tracer.h"
#include "scene/rt_camera.h"

RTRenderer::RTRenderer( const RTRenderSettings &settings )
    : Settings( settings )
    , RenderTime( 0.0 )
{
}

RTImage RTRenderer::render( RTScene *scene, const RTCamera &camera )
{
    const int width  = Settings.Width;
    const int height = Settings.Height;
    RTImage image( width, height );

    RTCamera imageCamera( camera );
    imageCamera.setScreenSize( width, height );

    // Tiles are clipped at the right and bottom edge, so any image size works
    RTImageParts parts;
    const int tileWidth  = std::max( 1, Settings.TileWidth );
    const int tileHeight = std::max( 1, Settings.TileHeight );
    for ( int row = 0; row < height; row += tileHeight )
    {
        for ( int col = 0; col < width; col += tileWidth )
        {
            parts.addPart( row, col, std::min( row + tileHeight, height ), std::min( col + tileWidth, width ) );
        }
    }

    std::vector<RTTracer> tracers( std::max( 1, Settings.NumberOfThreads ) );
    for ( RTTracer &tracer : tracers )
    {
        tracer.init( scene, &imageCamera, &image, &parts, Settings.UseSIMD );
        tracer.setMaxTraceDepth( Settings.MaxTraceDepth );
    }

    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

    std::vector<std::thread> threads;
    for ( RTTracer &tracer : tracers )
    {
        threads.push_back( std::thread( &RTTracer::render, &tracer ) );
    }
    for ( std::thread &thread : threads )
    {
        thread.join();
    }

    RenderTime = std::chrono::duration<double>( std::chrono::steady_clock::now() - start ).count();

    return image;
}

double RTRenderer::getRenderTime()
{
    return RenderTime;
}
