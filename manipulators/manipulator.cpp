#include <QDebug>
#include <QtMath>
#include "manipulator.h"

Manipulator::Manipulator()
 : ActiveObject(0)
{
    Plane.normal = QVector3D( 0.0f, 1.0f, 0.0f );
    m_ptrManipulate = &Manipulator::translateXZ;
}

void Manipulator::initialize( QVector3D hitpoint )
{
    Plane.position = hitpoint;
    Translation = QVector3D( 0.0f, 0.0f, 0.0f );
}

void Manipulator::update( const RTRay &ray )
{
    QVector3D newHitPoint;
    intersection( ray, newHitPoint );
    Translation = newHitPoint - Plane.position;
    Plane.position = newHitPoint;
    (this->*m_ptrManipulate)();
}

void Manipulator::setActiveObject( RTObject *object )
{
    noActiveLight();
    ActiveObject = object;
}

void Manipulator::noActiveObject()
{
    ActiveObject = nullptr;
}

void Manipulator::setActiveLight( RTLight *light )
{
    noActiveObject();
    ActiveLight = light;
}

void Manipulator::noActiveLight()
{
    ActiveLight = nullptr;
}

QVector3D Manipulator::getTranslation() const
{
    return Translation;
}

void Manipulator::constraintXZ()
{
    Plane.normal = QVector3D( 0.0f, 1.0f, 0.0f );
    m_ptrManipulate = &Manipulator::translateXZ;
}

void Manipulator::constraintXY()
{
    Plane.normal = QVector3D( 0.0f, 0.0f, 1.0f );
    m_ptrManipulate = &Manipulator::translateXY;
}

void Manipulator::constraintYZ()
{
    Plane.normal = QVector3D( 1.0f, 0.0f, 0.0f );
    m_ptrManipulate = &Manipulator::translateYZ;
}

bool Manipulator::intersection( const RTRay &ray, QVector3D &hitPoint )
{
    QVector3D d( ray.Direction.x(), ray.Direction.y(), ray.Direction.z() );
    float den = QVector3D::dotProduct( Plane.normal, d );

    if( fabs( den ) < 1e-6 )
    {
        return false;
    }
    QVector3D o( ray.Origin.x(), ray.Origin.y(), ray.Origin.z() );
    float num = QVector3D::dotProduct( ( o - Plane.position ), Plane.normal );
    float dist = -( num / den );
    if ( dist < 0 )
    {
        return false;
    }
    RTVector hit = ray.Origin + ray.Direction * dist;
    hitPoint = QVector3D( hit.x(), hit.y(), hit.z() );
    return true;
}

void Manipulator::translateXZ()
{
    if( ActiveObject != nullptr )
    {
        ActiveObject->movePosition( RTVector(
                                         Translation.x(),
                                         0.0f,
                                         Translation.z()
                                     ) );
    }
    else if ( ActiveLight != nullptr )
    {
        ActiveLight->movePosition( RTVector(
                                       Translation.x(),
                                       0.0f,
                                       Translation.z()
                                    ) );
    }
}

void Manipulator::translateXY()
{
    if( ActiveObject != nullptr )
    {
        ActiveObject->movePosition( RTVector(
                                         Translation.x(),
                                         Translation.y(),
                                         0.0f
                                     ) );
    }
    else if ( ActiveLight != nullptr )
    {
        ActiveLight->movePosition( RTVector(
                                         Translation.x(),
                                         Translation.y(),
                                         0.0f
                                     ) );
    }
}

void Manipulator::translateYZ()
{
    if( ActiveObject != nullptr )
    {
        ActiveObject->movePosition( RTVector(
                                         0.0f,
                                         Translation.y(),
                                         Translation.z()
                                     ) );
    }
    else if ( ActiveLight != nullptr )
    {
        ActiveLight->movePosition( RTVector(
                                         0.0f,
                                         Translation.y(),
                                         Translation.z()
                                     ) );
    }
}
