#ifndef RT_LIGHT_H
#define RT_LIGHT_H

#include <memory>
#include <string>
#include "../raytracer/rt_ray.h"
#include "math/rt_vector.h"

class GLObject;

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
        return Globject.get();
    }

    void   setGLObject( std::shared_ptr<GLObject> glObject );

    void   updatePosition( RTVector position );
    void   movePosition( RTVector movementStep );
    void   setScale( RTVector scale );

    void setName( const std::string &name );
    void setColor( RTVector color );
    void setIntensity( float intensity );

    // The direction the light travels in at hitPoint, its intensity there, and its distance from hitPoint:
    // only objects closer than that cast a shadow (the largest float for a distant light)
    virtual void illuminate( const RTVector &hitPoint, RTVector &lightDirection, RTVector &intensity, float &distance ) const = 0;

    RTVector  getPosition();
    RTVector  getScale();
    RTVector  getColor();
    float       getIntensity();
    std::string getName();

protected:
    std::string Name;
    RTVector Position;
    RTVector Scale;
    RTVector Color;
    float    Intensity;

private:
    // See RTObject::Globject
    std::shared_ptr<GLObject> Globject;
};

#endif // RT_LIGHT_H
