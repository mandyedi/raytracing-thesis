#include <QDebug>
#include "rt_point_light.h"

RTPointLight::RTPointLight( RTVector position, RTVector color, float intensity )
    : RTLight( position, color, intensity )
{
}

RTPointLight::~RTPointLight()
{
}

int RTPointLight::intersect( const RTRay &ray, float &distance )
{
    RTVector v = ray.Origin - Position;
    float b = -RTVector::DotProduct(v, ray.Direction);
    float r = 0.05f;
    float det = ( b * b ) - RTVector::DotProduct( v, v ) + ( r * r );
    int retval = 0;
    if ( det > 0 )
    {
        det = sqrtf( det );
        float t0 = b - det;
        float t1 = b + det;

        if ( t0 > t1 )
        {
            std::swap( t0, t1 );
        }

        if ( t0 < 0 )
        {
            t0 = t1;
            if (t0 < 0)
            {
                return 0;
            }
        }

        distance = t0;

        return 1;
    }
    return retval;
}

void RTPointLight::illuminate( const RTVector &hitPoint, RTVector &lightDirection, RTVector &intensity ) const
{
    lightDirection = hitPoint - Position;
    lightDirection.Normalize();
    intensity = Intensity * Color;
}
