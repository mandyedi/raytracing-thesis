#include "rt_light.h"

RTLight::RTLight( RTVector position, RTVector color, float intensity )
 : Name( "no name" )
 , Position( position )
 , Scale( RTVector( 0.05f, 0.05f, 0.05f ) )
 , Color( color )
 , Intensity( intensity )
 , Globject( nullptr )
{
}

RTLight::~RTLight()
{
}

void RTLight::setGLObject( std::shared_ptr<GLObject> glObject )
{
    Globject = glObject;
}

void RTLight::updatePosition( RTVector position )
{
    Position = position;
}

void RTLight::movePosition( RTVector movementStep )
{
    Position += movementStep;
}

void RTLight::setScale( RTVector scale )
{
    Scale = scale;
}

void RTLight::setName( const std::string &name )
{
    Name = name;
}

void RTLight::setColor( RTVector color )
{
    Color = color;
}

void RTLight::setIntensity( float intensity )
{
    Intensity = intensity;
}

RTVector RTLight::getPosition()
{
    return  Position;
}

RTVector RTLight::getScale()
{
    return  Scale;
}

RTVector RTLight::getColor()
{
    return Color;
}

float RTLight::getIntensity()
{
    return Intensity;
}

std::string RTLight::getName()
{
    return  Name;
}
