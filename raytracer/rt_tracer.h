#ifndef RT_TRACER_H
#define RT_TRACER_H

#include <QObject>
#include <QVector3D>
#include "rt_ray.h"
#include "../rt_scene.h"
#include "../rt_camera.h"
#include "rt_image_parts.h"
#include "math/rt_vector.h"

class RTTracer : public QObject
{
    Q_OBJECT

public:
    RTTracer( QObject *parent = 0 );
    void init( RTScene *scene, RTCamera *camera, RTVector **pixels, RTImageParts *imageParts, bool useSIMD );
    void setMaxTraceDepth( int maxTraceDepth );

public slots:
    void render();

signals:
    void finished();
    void updateTimer();

public:
    void castRay( RTRay &ray, RTVector &color, const int &depth );

private:
    RTVector reflect( const RTVector &incident, const RTVector &reflected );

private:
    RTScene       *Scene;
    RTCamera      *Camera;
    RTVector     **Pixels;
    RTImageParts  *ImageParts;
    int            MaxTraceDepth;
    bool           UseSIMD;

    RTObject* Trace( RTRay &ray, float &distance, float &u, float &v, unsigned int &triangleIndex );

    // Examples
    void castIntersection( RTRay &ray, RTVector &color );
};

#endif // RT_TRACER_H
