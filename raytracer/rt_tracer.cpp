#include <iostream>
#include <QDebug>
#include <QtMath>

#include "rt_tracer.h"
#include "../objects/rt_object.h"
#include "../objects/rt_point_light.h"
#include "../objects/rt_distant_light.h"

using namespace  std;

RTTracer::RTTracer( QObject *parent )
: QObject(parent)
{
}

void RTTracer::init( RTScene *scene, RTCamera *camera, RTVector **pixels, RTImageParts *imageParts, bool useSIMD )
{
    Scene         = scene;
    Camera        = camera;
    Pixels        = pixels;
    ImageParts    = imageParts;
    MaxTraceDepth = 1;
    UseSIMD       = useSIMD;
}

void RTTracer::setMaxTraceDepth( int maxTraceDepth )
{
    qDebug() << "RTTracer::setMaxTraceDepth value: " << maxTraceDepth;
    MaxTraceDepth = maxTraceDepth;
}

void RTTracer::render()
{
    while( !ImageParts->isEmpty() )
    {
        PART  p    = ImageParts->getPart();
        float rowf = static_cast<float>( p.startRow );
        float colf = static_cast<float>( p.startCol );
        QVector3D rayOrigin;
        Camera->setRayOrigin( rayOrigin );

        for ( unsigned int row = p.startRow; row < p.endRow; row++ )
        {
            colf = static_cast<float>( p.startCol );
            for ( unsigned int col = p.startCol; col < p.endCol; col++ )
            {
                RTRay ray;
                Camera->setRayDirection( rowf, colf, ray );

                RTVector color( 0.0f, 0.0f, 0.0f );
                castRay( ray, color, 0 );

                Pixels[row][col] = color;

                colf++;
            }

            rowf++;
        }
    }

    emit updateTimer();
    emit finished();
}

RTObject* RTTracer::Trace( RTRay &ray, float &distance, float &u, float &v, unsigned int &triangleIndex )
{
    RTObject *object = nullptr;
    for ( unsigned int i = 0; i < Scene->getNumberOfObjects(); i++ )
    {
        RTObject *prTemp = Scene->getObject(i);
        float intersectionDistance = std::numeric_limits<float>::max();
        unsigned int index;
        float tempU;
        float tempV;
        if ( prTemp->intersect( ray, intersectionDistance, index, tempU, tempV, UseSIMD ) && intersectionDistance < distance )
        {
            distance = intersectionDistance;
            object = prTemp;
            triangleIndex = index;
            u = tempU;
            v = tempV;
        }
    }
    return object;
}

void RTTracer::castRay( RTRay &_ray, RTVector &_color, const int &_depth )
{
    if ( _depth >= MaxTraceDepth )
    {
        return;
    }

    float distance = std::numeric_limits<float>::max();
    unsigned int triangleIndex = UINT_MAX;
    float u, v;
    RTObject *object = Trace( _ray, distance, u, v, triangleIndex );

    if ( object )
    {
        RTVector hitPoint  = _ray.Origin + _ray.Direction * distance;
        RTVector hitNormal = object->getNormal( triangleIndex, u, v );

        RTVector hitNormals[3];
        object->getNormalsForSmooth( triangleIndex, hitNormals[0], hitNormals[1], hitNormals[2] );

        if ( object->getMaterialType() == RTObject::RTMaterialType::Diffuse )
        {
            RTVector lightDirection;
            RTVector intensity;

            for ( size_t i = 0; i < Scene->getNumberOfLights(); i++ )
            {
                RTLight *light = Scene->getLight( i );
                light->illuminate( hitPoint, lightDirection, intensity );

                // Check shadow then shade
                float shadowBias = 0.0001f;     // To avoid shadow-acne (self intersection)
                float distanceShadow = std::numeric_limits<float>::max();
                unsigned int triangleIndex = UINT_MAX;
                RTRay shadowRay( hitPoint + hitNormal * shadowBias, -lightDirection );
                float uShadow, vShadow;
                RTObject *shadowObject = Trace( shadowRay, distanceShadow, uShadow, vShadow, triangleIndex );
                float shadow = shadowObject == nullptr ? 1.f : 0.f;

                // Diffuse component
                float dot = 0.f;
                if ( object->getSmoothShading() )
                {
                    float dots[3];
                    for ( unsigned int i = 0; i < 3; i++ )
                    {
                        dots[i] = RTVector::DotProduct( hitNormals[i], -lightDirection );
                    }
                    dot = ( 1 - u - v ) * dots[0] + u * dots[1] + v * dots[2];
                } else {
                    dot = RTVector::DotProduct( hitNormal, -lightDirection );
                }
                _color += shadow * object->getColor() * intensity * std::max( 0.0f, dot );
            }
        }
        else if ( object->getMaterialType() == RTObject::RTMaterialType::Specular )
        {
            RTVector lightDirection;
            RTVector intensity;

            for ( size_t i = 0; i < Scene->getNumberOfLights(); i++ )
            {
                RTLight *light = Scene->getLight( i );
                light->illuminate( hitPoint, lightDirection, intensity );

                // Check shadow then shade
                float shadowBias = 0.0001f;     // To avoid shadow-acne (self intersection)
                float distanceShadow = std::numeric_limits<float>::max();
                unsigned int triangleIndex = UINT_MAX;
                RTRay shadowRay( hitPoint + hitNormal * shadowBias, -lightDirection );
                float uShadow, vShadow;
                RTObject *shadowObject = Trace( shadowRay, distanceShadow, uShadow, vShadow, triangleIndex );
                float shadow = shadowObject == nullptr ? 1.f : 0.f;

                // Specular component
                RTVector R;
                if ( object->getSmoothShading() )
                {
                    RTVector Rs[3];
                    for ( unsigned int i = 0; i < 3; i++ )
                    {
                        Rs[i] = reflect( lightDirection, hitNormals[i] );
                    }

                    R = ( 1 - u - v ) * Rs[0] + u * Rs[1] + v * Rs[2];
                } else {
                    R = reflect( lightDirection, hitNormal );
                }
                _color += shadow * intensity * std::pow( std::max( 0.f, RTVector::DotProduct( R, -_ray.Direction ) ), object->getSpecularExponent() );
            }
        }
        else if ( object->getMaterialType() == RTObject::RTMaterialType::DiffuseAndSpecular )
        {
            RTVector lightDirection;
            RTVector intensity;

            RTVector diffuse( 0.0f, 0.0f, 0.0f );
            RTVector specular( 0.0f, 0.0f, 0.0f );

            for ( size_t i = 0; i < Scene->getNumberOfLights(); i++ )
            {
                RTLight *light = Scene->getLight( i );
                light->illuminate( hitPoint, lightDirection, intensity );

                // Check shadow then shade
                float shadowBias = 0.0001f;     // To avoid shadow-acne (self intersection)
                float distanceShadow = std::numeric_limits<float>::max();
                unsigned int triangleIndex = UINT_MAX;
                RTRay shadowRay( hitPoint + hitNormal * shadowBias, -lightDirection );
                float uShadow, vShadow;
                RTObject *shadowObject = Trace( shadowRay, distanceShadow, uShadow, vShadow, triangleIndex );
                float shadow = shadowObject == nullptr ? 1.f : 0.f;

                // Diffuse component
                float dot = 0.f;
                if ( object->getSmoothShading() )
                {
                    float dots[3];
                    for ( unsigned int i = 0; i < 3; i++ )
                    {
                        dots[i] = RTVector::DotProduct(hitNormals[i], -lightDirection);
                    }
                    dot = ( 1 - u - v ) * dots[0] + u * dots[1] + v * dots[2];
                } else {
                    dot = RTVector::DotProduct(hitNormal, -lightDirection);
                }
                diffuse += shadow * object->getColor() * intensity * std::max(0.0f, dot);

                // Specular component
                RTVector R;
                if ( object->getSmoothShading() )
                {
                    RTVector Rs[3];
                    for ( unsigned int i = 0; i < 3; i++ )
                    {
                        Rs[i] = reflect( lightDirection, hitNormals[i] );
                    }

                    R = ( 1 - u - v ) * Rs[0] + u * Rs[1] + v * Rs[2];
                } else {
                    R = reflect( lightDirection, hitNormal );
                }
                specular += shadow * intensity * std::pow( std::max( 0.f, RTVector::DotProduct( R, -_ray.Direction ) ), object->getSpecularExponent() );
                _color += diffuse * object->getDiffuse() + specular * object->getSpecular();
            }
        }
        else if ( object->getMaterialType() == RTObject::RTMaterialType::Reflective )
        {
            RTVector R = reflect( _ray.Direction, hitNormal );
            R.Normalize();
            float bias = 0.001f;
            RTVector reflectionColor( 0.0f, 0.0f, 0.0f );
            RTRay reflectionRay( hitPoint + hitNormal * bias, R );
            castRay( reflectionRay, reflectionColor, _depth + 1 );
            _color = object->getReflection() * reflectionColor;
        }
    }
    else
    {
        _color = RTVector( 0.18f, 0.18f, 0.18f );
    }
}

RTVector RTTracer::reflect( const RTVector &incident, const RTVector &normal )
{
    return incident - 2.0f * RTVector::DotProduct( incident, normal ) * normal;
}

// Examples
void RTTracer::castIntersection( RTRay &ray, RTVector &color )
{
    float distance = std::numeric_limits<float>::max();
    unsigned int triangleIndex = std::numeric_limits<unsigned int>::max();
    float u, v;
    RTObject *object = Trace( ray, distance, u, v, triangleIndex );
    if ( object )
    {
        color = object->getColor();
    }
}
