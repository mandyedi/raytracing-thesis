#include <QtMath>
#include "rt_camera.h"

RTCamera::RTCamera()
{
    Up  = QVector3D( 0.0, 1.0, 0.0 );
    FOV = static_cast<float>( tan( 45.0 * 0.5 * M_PI / 180.0 ) );
    AngleH = 3.14f;
    AngleV = 0.0f;

    Zoom = 4;
}

void RTCamera::moveCamera( const QVector3D &direction )
{
    Eye += direction;
    At  += direction;

    qDebug() << "Camera Eye: " << Eye;
}

void RTCamera::rotateCamera( const float &h, const float &v )
{
    AngleH += h;
    AngleV += v;

    // Kamera nezopont iranya
    QVector3D direction(
                cosf( AngleV ) * sinf( AngleH ),
                sinf( AngleV ),
                cosf( AngleV ) * cosf( AngleH )
    );
    direction.normalize();
    At = Eye + direction;

    QVector3D right(
                sinf( AngleH - 3.14f / 2.0f ),
                0.0f,
                cosf( AngleH - 3.14f / 2.0f )
    );
    Up = QVector3D::crossProduct( right, direction );
}

void RTCamera::setScreenSize( const int &width, const int &height )
{
    ScreenWidth  = width;
    ScreenHeight = height;
    AspectRatio  = static_cast<float>(width)/static_cast<float>(height);
}

void RTCamera::setRayOrigin( QVector3D &origin )
{
    QMatrix4x4 mat;
    mat.setToIdentity();
    mat.translate( Eye );
    origin = mat * origin;
}

QVector3D RTCamera::getEye()
{
    return Eye;
}

QVector3D RTCamera::getAt()
{
    return At;
}

QVector3D RTCamera::getUp()
{
    return Up;
}

int RTCamera::getScreenHeight()
{
    return ScreenHeight;
}

int RTCamera::getScreenWidth()
{
    return ScreenWidth;
}
