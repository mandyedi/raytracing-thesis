#ifndef RT_POINT_LIGHT_H
#define RT_POINT_LIGHT_H

#include "rt_light.h"

class RTPointLight : public RTLight
{
public: // functions
    RTPointLight( RTVector position, RTVector color, float intensity );
    ~RTPointLight();

    RTLightType getLightType()
    {
        return RTLightType::Point;
    }

    int intersect( const RTRay &ray, float &distance );
    void illuminate( const RTVector &hitPoint, RTVector &lightDirection, RTVector &intensity ) const;
};

#endif // RT_POINT_LIGHT_H
