#ifndef RT_CAMERA_H
#define RT_CAMERA_H

#include <QVector3D>
#include <QMatrix4x4>
#include "raytracer/rt_ray.h"

enum RTCameraType
{
    RTCameraTypePerspective,
    RTCameraTypeOrtho
};

enum RTCameraView
{
    RTCameraViewNone,
    RTCameraViewTop,
    RTCameraViewFront,
    RTCameraViewRight
};

class RTCamera
{
public:
    RTCamera();

    void moveCamera( const QVector3D &direction );
    void rotateCamera( const float &h, const float &v );

    void setScreenSize( const int &width, const int &height );
    void setRayOrigin( QVector3D &origin );

    void setCamera( QVector3D eye, QVector3D at, RTCameraType cameraType, RTCameraView cameraView )
    {
        Eye = eye;
        At  = at;
        CameraType = cameraType;
        CameraView = cameraView;
    }

    QVector3D getEye();
    QVector3D getAt();
    QVector3D getUp();

    int   getScreenHeight();
    int   getScreenWidth();

    inline RTCameraView getCameraView()
    {
        return CameraView;
    }

    inline QMatrix4x4& getProjectionMatrix()
    {
        ProjectionMatrix.setToIdentity();

        if ( CameraType == RTCameraTypePerspective )
        {
            ProjectionMatrix.perspective(
                        45.0f,
                        static_cast<qreal>(ScreenWidth) / static_cast<qreal>(ScreenHeight),
                        0.1f,
                        20.0f);
        }
        else if ( CameraType == RTCameraTypeOrtho )
        {
            ProjectionMatrix.ortho( -AspectRatio * Zoom, AspectRatio * Zoom, -Zoom, Zoom, 0.1, 10 );
        }

        return ProjectionMatrix;
    }

    void setRayDirection( const float row, const float col, RTRay &ray )
    {
        if ( CameraType == RTCameraTypePerspective )
        {
            float xx = ( 2 * ( col + 0.5 ) / ScreenWidth - 1 ) * FOV * AspectRatio;
            float yy = ( 1 - 2 * ( row + 0.5 ) / ScreenHeight ) * FOV;

            QVector3D forward = At-Eye;
            forward.normalize();
            QVector3D right = QVector3D::crossProduct( forward, Up );

            ray.Direction.setX( right.x() * xx + Up.x() * yy + forward.x() );
            ray.Direction.setY( right.y() * xx + Up.y() * yy + forward.y() );
            ray.Direction.setZ( right.z() * xx + Up.z() * yy + forward.z() );
            ray.Direction.Normalize();

            ray.Origin.setX( Eye.x() );
            ray.Origin.setY( Eye.y() );
            ray.Origin.setZ( Eye.z() );
        }
        else if ( CameraType == RTCameraTypeOrtho )
        {
            float xx = ( Zoom * AspectRatio * 2 * col / ScreenWidth ) - Zoom * AspectRatio;
            float yy = Zoom - ( Zoom * 2 * row / ScreenHeight );

            QVector3D direction = At - Eye;
            ray.Direction.setX( direction.x() );
            ray.Direction.setY( direction.y() );
            ray.Direction.setZ( direction.z() );
            ray.Direction.Normalize();

            if ( CameraView == RTCameraViewFront )
            {
                ray.Origin.setX( xx );
                ray.Origin.setY( yy );
                ray.Origin.setZ( 100.0f );
            }
            else if ( CameraView == RTCameraViewTop )
            {
                ray.Origin.setX( xx );
                ray.Origin.setY( 100.0f );
                ray.Origin.setZ( -yy );
            }
            else if ( CameraView == RTCameraViewRight )
            {
                ray.Origin.setX( 100.0f );
                ray.Origin.setY( yy );
                ray.Origin.setZ( -xx );
            }
        }
    }

private:
    QVector3D  Eye;
    QVector3D  At;
    QVector3D  Up;
    QMatrix4x4 ProjectionMatrix;

    float AngleH;
    float AngleV;

    int   ScreenWidth;
    int   ScreenHeight;
    float FOV;
    float AspectRatio;
    float Zoom;

    RTCameraType CameraType;
    RTCameraView CameraView;
};

#endif // RT_CAMERA_H
