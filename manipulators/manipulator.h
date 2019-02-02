#ifndef MANIPULATOR_H
#define MANIPULATOR_H

#include <QObject>
#include <QVector3D>
#include "../raytracer/rt_ray.h"
#include "objects/rt_object.h"
#include "objects/rt_light.h"

class Manipulator : public QObject
{
    Q_OBJECT
public:
    Manipulator();

    void initialize( QVector3D hitpoint );
    void update( const RTRay &ray );
    void setActiveObject( RTObject *object );
    void noActiveObject();
    void setActiveLight( RTLight *light );
    void noActiveLight();

    QVector3D getTranslation() const;

private slots:
    void constraintXZ();
    void constraintXY();
    void constraintYZ();

private:
    bool intersection( const RTRay &_ray, QVector3D &_hitPoint );
    void translateXZ();
    void translateXY();
    void translateYZ();

private:
    struct sPlane
    {
        QVector3D position;
        QVector3D normal;
    } Plane;

    QVector3D  Translation;
    RTObject  *ActiveObject;
    RTLight   *ActiveLight;

    void (Manipulator::*m_ptrManipulate)(void);
};

#endif // MANIPULATOR_H
