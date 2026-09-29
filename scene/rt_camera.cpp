#include <cmath>
#include "rt_camera.h"
#include "math/utils.h"

// Defaults to the GUI's start camera, so a scene file without a camera line still renders
RTCamera::RTCamera()
    : Eye( -0.8f, 1.6f, 6.0f )
    , At( 0.0f, 0.0f, 0.0f )
    , Up( 0.0f, 1.0f, 0.0f )
    , AngleH( 3.14f )
    , AngleV( 0.0f )
    , FieldOfView( 45.0f )
    , FOV( static_cast<float>( tan( 45.0 * 0.5 * Utils::Pi / 180.0 ) ) )
    , Zoom( 4 )
    , CameraType( RTCameraTypePerspective )
    , CameraView( RTCameraViewNone )
{
    setScreenSize( 800, 600 );
    updateForwardAndRight();
}

void RTCamera::moveCamera( const RTVector &direction )
{
    Eye += direction;
    At  += direction;
    updateForwardAndRight();
}

void RTCamera::rotateCamera( const float &h, const float &v )
{
    AngleH += h;
    AngleV += v;

    // Kamera nezopont iranya
    RTVector direction(
                cosf( AngleV ) * sinf( AngleH ),
                sinf( AngleV ),
                cosf( AngleV ) * cosf( AngleH )
    );
    normalize( direction );
    At = Eye + direction;

    RTVector right(
                sinf( AngleH - 3.14f / 2.0f ),
                0.0f,
                cosf( AngleH - 3.14f / 2.0f )
    );
    Up = RTVector::CrossProduct( right, direction );
    updateForwardAndRight();
}

void RTCamera::updateForwardAndRight()
{
    Forward = At - Eye;
    normalize( Forward );
    Right = RTVector::CrossProduct( Forward, Up );
}

void RTCamera::setScreenSize( const int &width, const int &height )
{
    ScreenWidth  = width;
    ScreenHeight = height;
    AspectRatio  = static_cast<float>(width)/static_cast<float>(height);
}

RTVector RTCamera::getEye()
{
    return Eye;
}

RTVector RTCamera::getAt()
{
    return At;
}

RTVector RTCamera::getUp()
{
    return Up;
}

void RTCamera::setUp( const RTVector &up )
{
    Up = up;
    updateForwardAndRight();
}

void RTCamera::setFieldOfView( float degrees )
{
    FieldOfView = degrees;
    FOV = static_cast<float>( tan( degrees * 0.5 * Utils::Pi / 180.0 ) );
}

float RTCamera::getFieldOfView()
{
    return FieldOfView;
}

int RTCamera::getScreenHeight()
{
    return ScreenHeight;
}

int RTCamera::getScreenWidth()
{
    return ScreenWidth;
}

float RTCamera::getAspectRatio()
{
    return AspectRatio;
}

float RTCamera::getZoom()
{
    return Zoom;
}
