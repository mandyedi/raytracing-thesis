#ifndef RT_LIGHT_H
#define RT_LIGHT_H

#include <QString>
#include <QGLBuffer>
#include <QTextStream>
#include "../raytracer/rt_ray.h"
#include "../opengl/gl_object.h"
#include "math/rt_vector.h"

class RTLight
{
public:
    RTLight( RTVector position, RTVector color, float intensity );
    virtual ~RTLight();

    enum class RTLightType { Point = 0, Distant, Size };

    virtual RTLightType getLightType()= 0;

    virtual int intersect( const RTRay &ray, float &distance ) = 0;

    inline GLObject* getGlObject()
    {
        return Globject;
    }

    void   setGLObject( GLObject *glObject );

    void   updatePosition( RTVector position );
    void   movePosition( RTVector movementStep );
    void   setScale( RTVector scale );

    void setName( QString name );

    virtual void illuminate( const RTVector &hitPoint, RTVector &lightDirection, RTVector &intensity ) const = 0;

    RTVector  getPosition();
    RTVector  getScale();
    RTVector  getColor();
    float     getIntensity();
    QString   getName();

protected:
    QString  Name;
    RTVector Position;
    RTVector Scale;
    RTVector Color;
    float    Intensity;

private:
    GLObject *Globject;
};

#endif // RT_LIGHT_H
