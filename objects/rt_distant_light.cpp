#include "rt_distant_light.h"

RTDistantLight::RTDistantLight( RTVector direction, RTVector color, float intensity )
    : RTLight( RTVector( 0.0f, 0.0f, 0.0f ), color, intensity )
    , Direction( direction )
{
    Direction.Normalize();
}

RTDistantLight::~RTDistantLight(){}

int RTDistantLight::intersect( const RTRay &ray, float &distance )
{
    return 0;
}

void RTDistantLight::illuminate( const RTVector &hitPoint, RTVector &lightDirection, RTVector &intensity ) const
{
    lightDirection = Direction;
    intensity      = Color * Intensity;
}

void RTDistantLight::setDirectionX( float x )
{
    Direction.setX( x );
}

void RTDistantLight::setDirectionY( float y )
{
    Direction.setY( y );
}

void RTDistantLight::setDirectionZ( float z )
{
    Direction.setZ( z );
}
