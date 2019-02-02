#ifndef RT_DISTANTE_LIGHT_H
#define RT_DISTANTE_LIGHT_H

#include "rt_light.h"
#include "math/rt_vector.h"

class RTDistantLight : public RTLight
{
public:
    RTDistantLight( RTVector position, RTVector color, float intensity );
    ~RTDistantLight();

    RTLightType getLightType()
    {
        return RTLightType::Distant;
    }

    int intersect( const RTRay &ray, float &distance );
    void illuminate( const RTVector &hitPoint, RTVector &lightDirection, RTVector &intensity ) const;

    void setDirectionX( float x );
    void setDirectionY( float y );
    void setDirectionZ( float z );

private:
    RTVector Direction;
};

#endif // RT_DISTANTE_LIGHT_H
