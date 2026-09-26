#ifndef RT_TRACER_H
#define RT_TRACER_H

#include "rt_ray.h"
#include "../scene/rt_scene.h"
#include "../scene/rt_camera.h"
#include "rt_image.h"
#include "rt_image_parts.h"
#include "math/rt_vector.h"

class RTObject;

class RTTracer
{
public:
    RTTracer();
    void init( RTScene *scene, RTCamera *camera, RTImage *image, RTImageParts *imageParts, bool useSIMD );
    void setMaxTraceDepth( int maxTraceDepth );

    // Renders image parts until none are left; one call per render thread
    void render();

    void castRay( RTRay &ray, RTVector &color, const int &depth );

private:
    RTVector reflect( const RTVector &incident, const RTVector &reflected );

private:
    RTScene       *Scene;
    RTCamera      *Camera;
    RTImage       *Image;
    RTImageParts  *ImageParts;
    int            MaxTraceDepth;
    bool           UseSIMD;

    RTObject* Trace( RTRay &ray, float &distance, float &u, float &v, unsigned int &triangleIndex );

    // Examples
    void castIntersection( RTRay &ray, RTVector &color );
};

#endif // RT_TRACER_H
