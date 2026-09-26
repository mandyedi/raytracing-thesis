#ifndef RT_CAMERA_H
#define RT_CAMERA_H

#include <cmath>
#include "math/rt_vector.h"
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

    void moveCamera( const RTVector &direction );
    void rotateCamera( const float &h, const float &v );

    void setScreenSize( const int &width, const int &height );

    void setCamera( RTVector eye, RTVector at, RTCameraType cameraType, RTCameraView cameraView )
    {
        Eye = eye;
        At  = at;
        CameraType = cameraType;
        CameraView = cameraView;
    }

    RTVector getEye();
    RTVector getAt();
    RTVector getUp();

    int   getScreenHeight();
    int   getScreenWidth();
    float getAspectRatio();
    float getZoom();

    inline RTCameraType getCameraType()
    {
        return CameraType;
    }

    inline RTCameraView getCameraView()
    {
        return CameraView;
    }

    void setRayDirection( const float row, const float col, RTRay &ray )
    {
        if ( CameraType == RTCameraTypePerspective )
        {
            float xx = ( 2 * ( col + 0.5 ) / ScreenWidth - 1 ) * FOV * AspectRatio;
            float yy = ( 1 - 2 * ( row + 0.5 ) / ScreenHeight ) * FOV;

            RTVector forward = At - Eye;
            normalize( forward );
            RTVector right = RTVector::CrossProduct( forward, Up );

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

            RTVector direction = At - Eye;
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
    // Same math as QVector3D::normalize(), which the camera used before it became Qt-free:
    // the length is computed in double precision, so the rays stay exactly the same.
    static inline void normalize( RTVector &v )
    {
        double length = double( v.x() ) * double( v.x() ) +
                        double( v.y() ) * double( v.y() ) +
                        double( v.z() ) * double( v.z() );
        if ( std::fabs( length - 1.0f ) <= 0.000000000001 || std::fabs( length ) <= 0.000000000001 )
        {
            return;
        }

        length = std::sqrt( length );

        v.setX( float( double( v.x() ) / length ) );
        v.setY( float( double( v.y() ) / length ) );
        v.setZ( float( double( v.z() ) / length ) );
    }

    RTVector Eye;
    RTVector At;
    RTVector Up;

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
