// C API over the ray tracing core for the MCP server: mcp_server/server.py loads this library with ctypes,
// builds a scene with these calls and renders it whenever it wants to see the scene.
//
// - A scene is an opaque RTMcpScene*, from rt_create() until rt_destroy().
// - Functions return 1 on success and 0 on failure; those that return text return nullptr on failure.
//   rt_error() tells why. Returned text and image bytes stay valid until the next call for the same scene.
// - Objects and lights have unique names, and the caller refers to them by name. Every object is one of
//   the primitive meshes in the mesh folder.
// - Not thread-safe: calls for the same scene must not overlap.
// - C++ exceptions can't cross the C boundary, so every function catches them and fails instead.

#include <cmath>
#include <exception>
#include <limits>
#include <locale>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "objects/rt_distant_light.h"
#include "objects/rt_object.h"
#include "objects/rt_point_light.h"
#include "raytracer/rt_renderer.h"
#include "scene/rt_camera.h"
#include "scene/rt_scene.h"

#ifdef _WIN32
#define RT_API extern "C" __declspec( dllexport )
#else
#define RT_API extern "C" __attribute__( ( visibility( "default" ) ) )
#endif

struct RTMcpScene
{
    explicit RTMcpScene( const std::string &meshDirectory );

    RTCamera                   Camera;
    RTScene                    Scene;
    std::string                Error;       // why the last call failed
    std::string                Text;        // the name or JSON the last call returned
    std::vector<unsigned char> Image;       // PNG of the last render
    double                     RenderTime;  // seconds the last render took
    unsigned int               NameCount;   // numbers the generated names
};

namespace
{

const int MaxImageSide = 4096;
const int MaxDepth     = 10;

struct RTShape
{
    const char             *Name;
    RTObject::RTObjectType  Type;
    const char             *MeshFile;
};

const RTShape Shapes[] =
{
    { "plane",    RTObject::RTObjectType::Plane,    "plane.obj" },
    { "sphere",   RTObject::RTObjectType::Sphere,   "sphere.obj" },
    { "cube",     RTObject::RTObjectType::Cube,     "cube.obj" },
    { "pyramid",  RTObject::RTObjectType::Pyramid,  "pyramid.obj" },
    { "cylinder", RTObject::RTObjectType::Cylinder, "cylinder.obj" },
    { "cone",     RTObject::RTObjectType::Cone,     "cone.obj" },
    { "torus",    RTObject::RTObjectType::Tours,    "torus.obj" }
};

// In RTMaterialType order
const char *MaterialNames[] = { "diffuse", "specular", "diffuse_and_specular", "reflective" };
static_assert( sizeof( MaterialNames ) / sizeof( MaterialNames[0] ) == static_cast<size_t>( RTObject::RTMaterialType::Size ),
               "one name per material" );

std::string text( const char *value )
{
    return value != nullptr ? value : "";
}

// Records why the call failed. Returns false, so a check can end with return fail( ... ).
bool fail( RTMcpScene *scene, const std::string &message )
{
    scene->Error = message;
    return false;
}

// Called in a catch block, where a second exception must not escape
void failAfterException( RTMcpScene *scene, const char *what )
{
    try
    {
        scene->Error = std::string( "internal error: " ) + what;
    }
    catch ( ... )
    {
        scene->Error.clear();
    }
}

// Runs the body of an API function: 1 if it succeeds, 0 if it fails (after fail()) or throws
template <typename Body>
int call( RTMcpScene *scene, Body body )
{
    if ( scene == nullptr )
    {
        return 0;
    }
    try
    {
        return body() ? 1 : 0;
    }
    catch ( const std::exception &exception )
    {
        failAfterException( scene, exception.what() );
    }
    catch ( ... )
    {
        failAfterException( scene, "unknown exception" );
    }
    return 0;
}

// The same for an API function that returns the text the body puts in scene->Text
template <typename Body>
const char* callText( RTMcpScene *scene, Body body )
{
    return call( scene, body ) ? scene->Text.c_str() : nullptr;
}

bool isFinite( float x, float y, float z )
{
    return std::isfinite( x ) && std::isfinite( y ) && std::isfinite( z );
}

// Between 0 and 1; false for NaN
bool isUnit( float value )
{
    return value >= 0.0f && value <= 1.0f;
}

const RTShape* findShape( const std::string &name )
{
    for ( const RTShape &shape : Shapes )
    {
        if ( name == shape.Name )
        {
            return &shape;
        }
    }
    return nullptr;
}

const char* shapeName( RTObject::RTObjectType type )
{
    for ( const RTShape &shape : Shapes )
    {
        if ( type == shape.Type )
        {
            return shape.Name;
        }
    }
    return "obj";
}

RTObject* findObject( RTMcpScene *scene, const std::string &name )
{
    for ( RTObject *object : scene->Scene.getObjects() )
    {
        if ( object->getName() == name )
        {
            return object;
        }
    }
    return nullptr;
}

RTLight* findLight( RTMcpScene *scene, const std::string &name )
{
    for ( RTLight *light : scene->Scene.getLights() )
    {
        if ( light->getName() == name )
        {
            return light;
        }
    }
    return nullptr;
}

// 1 to 64 letters, digits, '_', '-' or '.', so a name never needs escaping in JSON
bool isValidName( const std::string &name )
{
    if ( name.empty() || name.size() > 64 )
    {
        return false;
    }
    for ( char c : name )
    {
        const bool valid = ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) ||
                           c == '_' || c == '-' || c == '.';
        if ( !valid )
        {
            return false;
        }
    }
    return true;
}

// The name of a new object or light: the requested one, which must be valid and unused,
// or without one a new name such as "sphere_3"
bool newName( RTMcpScene *scene, const char *requested, const std::string &prefix, std::string &name )
{
    if ( requested != nullptr && requested[0] != '\0' )
    {
        name = requested;
        if ( !isValidName( name ) )
        {
            return fail( scene, "invalid name '" + name + "': use 1 to 64 letters, digits, '_', '-' or '.'" );
        }
        if ( findObject( scene, name ) != nullptr || findLight( scene, name ) != nullptr )
        {
            return fail( scene, "the name '" + name + "' is already used" );
        }
        return true;
    }

    do
    {
        name = prefix + "_" + std::to_string( ++scene->NameCount );
    }
    while ( findObject( scene, name ) != nullptr || findLight( scene, name ) != nullptr );
    return true;
}

// The object an API call is about; fails the call if there is none
RTObject* objectNamed( RTMcpScene *scene, const char *name )
{
    const std::string key = text( name );
    RTObject *object = findObject( scene, key );
    if ( object == nullptr )
    {
        fail( scene, findLight( scene, key ) != nullptr ? "'" + key + "' is a light, not an object"
                                                        : "there is no object named '" + key + "'" );
    }
    return object;
}

RTLight* lightNamed( RTMcpScene *scene, const char *name )
{
    const std::string key = text( name );
    RTLight *light = findLight( scene, key );
    if ( light == nullptr )
    {
        fail( scene, findObject( scene, key ) != nullptr ? "'" + key + "' is an object, not a light"
                                                         : "there is no light named '" + key + "'" );
    }
    return light;
}

// Runs the body of an API function that changes the named object
template <typename Body>
int onObject( RTMcpScene *scene, const char *name, Body body )
{
    return call( scene, [&]() -> bool
    {
        RTObject *object = objectNamed( scene, name );
        return object != nullptr && body( object );
    } );
}

template <typename Body>
int onLight( RTMcpScene *scene, const char *name, Body body )
{
    return call( scene, [&]() -> bool
    {
        RTLight *light = lightNamed( scene, name );
        return light != nullptr && body( light );
    } );
}

// A distant light's direction, normalized
bool toDirection( RTMcpScene *scene, float x, float y, float z, RTVector &direction )
{
    const double length = std::sqrt( double( x ) * x + double( y ) * y + double( z ) * z );
    if ( !isFinite( x, y, z ) || !( length > 1e-6 ) )
    {
        return fail( scene, "the direction must be a finite vector that isn't 0" );
    }
    direction = RTVector( float( x / length ), float( y / length ), float( z / length ) );
    return true;
}

// Points the camera from eye at at. setCamera() keeps the camera's up at (0, 1, 0), which distorts tilted
// views (see RTCamera::setUp), so this also gives it an up vector perpendicular to the view: as close to
// +y as possible, and -z when looking straight up or down.
bool placeCamera( RTMcpScene *scene, const RTVector &eye, const RTVector &at, float fieldOfView )
{
    // In double precision, so even a nearly vertical view gets an exact perpendicular
    double dx = double( at.x() ) - eye.x();
    double dy = double( at.y() ) - eye.y();
    double dz = double( at.z() ) - eye.z();
    const double length = std::sqrt( dx * dx + dy * dy + dz * dz );
    if ( !( length > 1e-6 ) )
    {
        return fail( scene, "the camera's eye and look_at must be different points" );
    }
    dx /= length;
    dy /= length;
    dz /= length;

    // right = forward x world up, up = right x forward
    const bool vertical = std::fabs( dy ) > 0.9999;
    const double worldUpY = vertical ? 0.0 : 1.0;
    const double worldUpZ = vertical ? -1.0 : 0.0;
    double rx = dy * worldUpZ - dz * worldUpY;
    double ry = -dx * worldUpZ;
    double rz = dx * worldUpY;
    const double rightLength = std::sqrt( rx * rx + ry * ry + rz * rz );
    rx /= rightLength;
    ry /= rightLength;
    rz /= rightLength;
    const RTVector up( float( ry * dz - rz * dy ), float( rz * dx - rx * dz ), float( rx * dy - ry * dx ) );

    scene->Camera.setCamera( eye, at, RTCameraTypePerspective, RTCameraViewNone );
    scene->Camera.setUp( up );
    scene->Camera.setFieldOfView( fieldOfView );
    return true;
}

// JSON: numbers with the classic locale's '.', 6 significant digits like RTScene::saveScene.
// Names need no escaping, see isValidName().

void writeVector( std::ostream &out, const RTVector &v )
{
    out << '[' << v.x() << ", " << v.y() << ", " << v.z() << ']';
}

void writeCamera( std::ostream &out, RTCamera &camera )
{
    out << "{\"eye\": ";
    writeVector( out, camera.getEye() );
    out << ", \"look_at\": ";
    writeVector( out, camera.getAt() );
    out << ", \"fov\": " << camera.getFieldOfView() << '}';
}

void writeObject( std::ostream &out, RTObject *object )
{
    // World space bounding box, with the transform of RTObject::updateWorldSpace()
    const float inf = std::numeric_limits<float>::infinity();
    RTVector low( inf, inf, inf );
    RTVector high( -inf, -inf, -inf );
    const RTVector *vertices = object->getVertices();
    const RTVector scale     = object->getScale();
    const RTVector position  = object->getPosition();
    for ( unsigned int i = 0; i < object->getNumberOfVertices(); i++ )
    {
        RTVector v = ( vertices[i] * scale ) + position;
        low  = RTVector( std::fmin( low.x(), v.x() ), std::fmin( low.y(), v.y() ), std::fmin( low.z(), v.z() ) );
        high = RTVector( std::fmax( high.x(), v.x() ), std::fmax( high.y(), v.y() ), std::fmax( high.z(), v.z() ) );
    }

    out << "{\"name\": \"" << object->getName() << "\", \"shape\": \"" << shapeName( object->getObjectType() ) << "\"";
    out << ", \"position\": ";
    writeVector( out, object->getPosition() );
    out << ", \"scale\": ";
    writeVector( out, object->getScale() );
    out << ", \"color\": ";
    writeVector( out, object->getColor() );
    out << ", \"material\": \"" << MaterialNames[static_cast<int>( object->getMaterialType() )] << "\"";
    out << ", \"diffuse\": " << object->getDiffuse();
    out << ", \"specular\": " << object->getSpecular();
    out << ", \"specular_exponent\": " << object->getSpecularExponent();
    out << ", \"reflection\": " << object->getReflection();
    out << ", \"smooth_shading\": " << ( object->getSmoothShading() ? "true" : "false" );
    out << ", \"triangles\": " << object->getNumberOfVertices() / 3;
    out << ", \"bounds\": {\"min\": ";
    writeVector( out, low );
    out << ", \"max\": ";
    writeVector( out, high );
    out << "}}";
}

void writeLight( std::ostream &out, RTLight *light )
{
    out << "{\"name\": \"" << light->getName() << "\"";
    if ( RTDistantLight *distantLight = dynamic_cast<RTDistantLight*>( light ) )
    {
        out << ", \"type\": \"distant\", \"direction\": ";
        writeVector( out, distantLight->getDirection() );
    }
    else
    {
        out << ", \"type\": \"point\", \"position\": ";
        writeVector( out, light->getPosition() );
    }
    out << ", \"color\": ";
    writeVector( out, light->getColor() );
    out << ", \"intensity\": " << light->getIntensity() << '}';
}

} // namespace

RTMcpScene::RTMcpScene( const std::string &meshDirectory )
    : RenderTime( 0.0 )
    , NameCount( 0 )
{
    Scene.setCamera( &Camera );
    Scene.setMeshDirectory( meshDirectory );

    // The core's default view, upright
    placeCamera( this, Camera.getEye(), Camera.getAt(), Camera.getFieldOfView() );
}

// Scene

RT_API RTMcpScene* rt_create( const char *meshDirectory )
{
    try
    {
        return new RTMcpScene( text( meshDirectory ) );
    }
    catch ( ... )
    {
        return nullptr;
    }
}

RT_API void rt_destroy( RTMcpScene *scene )
{
    delete scene;
}

RT_API const char* rt_error( RTMcpScene *scene )
{
    return scene != nullptr ? scene->Error.c_str() : "no scene";
}

// Removes every object and light; the camera stays
RT_API int rt_clear( RTMcpScene *scene )
{
    return call( scene, [&]() -> bool
    {
        scene->Scene.removeAll();
        return true;
    } );
}

// Removes the object or light with this name
RT_API int rt_remove( RTMcpScene *scene, const char *name )
{
    return call( scene, [&]() -> bool
    {
        const std::string key = text( name );
        if ( scene->Scene.removeObject( key ) || scene->Scene.removeLight( key ) )
        {
            return true;
        }
        return fail( scene, "there is nothing named '" + key + "'" );
    } );
}

// {"camera": {...}, "objects": [...], "lights": [...]}, in the format of rt_item_json()
RT_API const char* rt_scene_json( RTMcpScene *scene )
{
    return callText( scene, [&]() -> bool
    {
        std::ostringstream out;
        out.imbue( std::locale::classic() );

        out << "{\"camera\": ";
        writeCamera( out, scene->Camera );

        out << ", \"objects\": [";
        const std::vector<RTObject*> &objects = scene->Scene.getObjects();
        for ( size_t i = 0; i < objects.size(); i++ )
        {
            out << ( i > 0 ? ", " : "" );
            writeObject( out, objects[i] );
        }

        out << "], \"lights\": [";
        const std::vector<RTLight*> &lights = scene->Scene.getLights();
        for ( size_t i = 0; i < lights.size(); i++ )
        {
            out << ( i > 0 ? ", " : "" );
            writeLight( out, lights[i] );
        }
        out << "]}";

        scene->Text = out.str();
        return true;
    } );
}

// The object or light with this name as JSON; an object includes its triangle count and world space bounds
RT_API const char* rt_item_json( RTMcpScene *scene, const char *name )
{
    return callText( scene, [&]() -> bool
    {
        const std::string key = text( name );
        std::ostringstream out;
        out.imbue( std::locale::classic() );

        if ( RTObject *object = findObject( scene, key ) )
        {
            writeObject( out, object );
        }
        else if ( RTLight *light = findLight( scene, key ) )
        {
            writeLight( out, light );
        }
        else
        {
            return fail( scene, "there is nothing named '" + key + "'" );
        }

        scene->Text = out.str();
        return true;
    } );
}

// Camera

// fieldOfView: vertical, in degrees
RT_API int rt_set_camera( RTMcpScene *scene, float eyeX, float eyeY, float eyeZ, float atX, float atY, float atZ, float fieldOfView )
{
    return call( scene, [&]() -> bool
    {
        if ( !isFinite( eyeX, eyeY, eyeZ ) || !isFinite( atX, atY, atZ ) )
        {
            return fail( scene, "the camera's eye and look_at need finite coordinates" );
        }
        if ( !( fieldOfView >= 1.0f && fieldOfView <= 170.0f ) )
        {
            return fail( scene, "the field of view must be between 1 and 170 degrees" );
        }
        return placeCamera( scene, RTVector( eyeX, eyeY, eyeZ ), RTVector( atX, atY, atZ ), fieldOfView );
    } );
}

// Objects

// Adds a shape (plane, sphere, cube, pyramid, cylinder, cone or torus) at the origin, with the core's default
// material. Without a name (nullptr or ""), it gets one like "sphere_3". Returns the name.
RT_API const char* rt_add_object( RTMcpScene *scene, const char *name, const char *shape )
{
    return callText( scene, [&]() -> bool
    {
        const RTShape *found = findShape( text( shape ) );
        if ( found == nullptr )
        {
            return fail( scene, "unknown shape '" + text( shape ) + "': use plane, sphere, cube, pyramid, cylinder, cone or torus" );
        }

        std::string objectName;
        if ( !newName( scene, name, found->Name, objectName ) )
        {
            return false;
        }

        std::unique_ptr<RTObject> object( new RTObject( scene->Scene.getMeshFile( found->MeshFile ), RTVector( 0.0f, 0.0f, 0.0f ) ) );
        if ( !object->isLoaded() )
        {
            return fail( scene, "cannot load mesh: " + object->getLoadError() );
        }
        object->setName( objectName );
        object->setObjectType( found->Type );

        // The scene owns it from here
        scene->Scene.addObject( object.get() );
        object.release();

        scene->Text = objectName;
        return true;
    } );
}

RT_API int rt_set_object_position( RTMcpScene *scene, const char *name, float x, float y, float z )
{
    return onObject( scene, name, [&]( RTObject *object ) -> bool
    {
        if ( !isFinite( x, y, z ) )
        {
            return fail( scene, "the position needs finite coordinates" );
        }
        object->updatePosition( RTVector( x, y, z ) );
        return true;
    } );
}

RT_API int rt_set_object_scale( RTMcpScene *scene, const char *name, float x, float y, float z )
{
    return onObject( scene, name, [&]( RTObject *object ) -> bool
    {
        if ( !isFinite( x, y, z ) || x == 0.0f || y == 0.0f || z == 0.0f )
        {
            return fail( scene, "the scale must be finite and not 0 on any axis" );
        }
        object->setScale( RTVector( x, y, z ) );
        return true;
    } );
}

RT_API int rt_set_object_color( RTMcpScene *scene, const char *name, float r, float g, float b )
{
    return onObject( scene, name, [&]( RTObject *object ) -> bool
    {
        if ( !isUnit( r ) || !isUnit( g ) || !isUnit( b ) )
        {
            return fail( scene, "color components must be between 0 and 1" );
        }
        object->setColor( RTVector( r, g, b ) );
        return true;
    } );
}

// material: diffuse, specular, diffuse_and_specular or reflective
RT_API int rt_set_object_material( RTMcpScene *scene, const char *name, const char *material )
{
    return onObject( scene, name, [&]( RTObject *object ) -> bool
    {
        const std::string wanted = text( material );
        for ( int i = 0; i < static_cast<int>( RTObject::RTMaterialType::Size ); i++ )
        {
            if ( wanted == MaterialNames[i] )
            {
                object->setMaterialType( static_cast<RTObject::RTMaterialType>( i ) );
                return true;
            }
        }
        return fail( scene, "unknown material '" + wanted + "': use diffuse, specular, diffuse_and_specular or reflective" );
    } );
}

// Weight of the diffuse part of diffuse_and_specular, 0 to 1
RT_API int rt_set_object_diffuse( RTMcpScene *scene, const char *name, float value )
{
    return onObject( scene, name, [&]( RTObject *object ) -> bool
    {
        if ( !isUnit( value ) )
        {
            return fail( scene, "diffuse must be between 0 and 1" );
        }
        object->setDiffuse( value );
        return true;
    } );
}

// Weight of the highlight of diffuse_and_specular, 0 to 1
RT_API int rt_set_object_specular( RTMcpScene *scene, const char *name, float value )
{
    return onObject( scene, name, [&]( RTObject *object ) -> bool
    {
        if ( !isUnit( value ) )
        {
            return fail( scene, "specular must be between 0 and 1" );
        }
        object->setSpecular( value );
        return true;
    } );
}

// Highlight sharpness of specular and diffuse_and_specular, above 0 and up to 10000
RT_API int rt_set_object_specular_exponent( RTMcpScene *scene, const char *name, float value )
{
    return onObject( scene, name, [&]( RTObject *object ) -> bool
    {
        if ( !( value > 0.0f && value <= 10000.0f ) )
        {
            return fail( scene, "specular_exponent must be above 0 and at most 10000" );
        }
        object->setSpecularExponent( value );
        return true;
    } );
}

// How much of the mirrored image a reflective object shows, 0 to 1
RT_API int rt_set_object_reflection( RTMcpScene *scene, const char *name, float value )
{
    return onObject( scene, name, [&]( RTObject *object ) -> bool
    {
        if ( !isUnit( value ) )
        {
            return fail( scene, "reflection must be between 0 and 1" );
        }
        object->setReflection( value );
        return true;
    } );
}

RT_API int rt_set_object_smooth_shading( RTMcpScene *scene, const char *name, int enabled )
{
    return onObject( scene, name, [&]( RTObject *object ) -> bool
    {
        object->setSmoothShading( enabled != 0 );
        return true;
    } );
}

// Lights

// Adds a white point light with intensity 1. Without a name, it gets one like "point_light_3". Returns the name.
RT_API const char* rt_add_point_light( RTMcpScene *scene, const char *name, float x, float y, float z )
{
    return callText( scene, [&]() -> bool
    {
        if ( !isFinite( x, y, z ) )
        {
            return fail( scene, "the position needs finite coordinates" );
        }

        std::string lightName;
        if ( !newName( scene, name, "point_light", lightName ) )
        {
            return false;
        }

        std::unique_ptr<RTLight> light( new RTPointLight( RTVector( x, y, z ), RTVector( 1.0f, 1.0f, 1.0f ), 1.0f ) );
        light->setName( lightName );
        scene->Scene.addLight( light.get() );
        light.release();

        scene->Text = lightName;
        return true;
    } );
}

// Adds a white distant light with intensity 1, shining in direction (normalized). Returns the name.
RT_API const char* rt_add_distant_light( RTMcpScene *scene, const char *name, float x, float y, float z )
{
    return callText( scene, [&]() -> bool
    {
        RTVector direction;
        if ( !toDirection( scene, x, y, z, direction ) )
        {
            return false;
        }

        std::string lightName;
        if ( !newName( scene, name, "distant_light", lightName ) )
        {
            return false;
        }

        std::unique_ptr<RTLight> light( new RTDistantLight( direction, RTVector( 1.0f, 1.0f, 1.0f ), 1.0f ) );
        light->setName( lightName );
        scene->Scene.addLight( light.get() );
        light.release();

        scene->Text = lightName;
        return true;
    } );
}

// Point lights only
RT_API int rt_set_light_position( RTMcpScene *scene, const char *name, float x, float y, float z )
{
    return onLight( scene, name, [&]( RTLight *light ) -> bool
    {
        if ( light->getLightType() != RTLight::RTLightType::Point )
        {
            return fail( scene, "'" + light->getName() + "' is a distant light: it has a direction, not a position" );
        }
        if ( !isFinite( x, y, z ) )
        {
            return fail( scene, "the position needs finite coordinates" );
        }
        light->updatePosition( RTVector( x, y, z ) );
        return true;
    } );
}

// Distant lights only; the direction is normalized
RT_API int rt_set_light_direction( RTMcpScene *scene, const char *name, float x, float y, float z )
{
    return onLight( scene, name, [&]( RTLight *light ) -> bool
    {
        RTDistantLight *distantLight = dynamic_cast<RTDistantLight*>( light );
        if ( distantLight == nullptr )
        {
            return fail( scene, "'" + light->getName() + "' is a point light: it has a position, not a direction" );
        }

        RTVector direction;
        if ( !toDirection( scene, x, y, z, direction ) )
        {
            return false;
        }
        distantLight->setDirectionX( direction.x() );
        distantLight->setDirectionY( direction.y() );
        distantLight->setDirectionZ( direction.z() );
        return true;
    } );
}

RT_API int rt_set_light_color( RTMcpScene *scene, const char *name, float r, float g, float b )
{
    return onLight( scene, name, [&]( RTLight *light ) -> bool
    {
        if ( !isUnit( r ) || !isUnit( g ) || !isUnit( b ) )
        {
            return fail( scene, "color components must be between 0 and 1" );
        }
        light->setColor( RTVector( r, g, b ) );
        return true;
    } );
}

RT_API int rt_set_light_intensity( RTMcpScene *scene, const char *name, float value )
{
    return onLight( scene, name, [&]( RTLight *light ) -> bool
    {
        if ( !( value >= 0.0f ) || !std::isfinite( value ) )
        {
            return fail( scene, "the intensity must be finite and not negative" );
        }
        light->setIntensity( value );
        return true;
    } );
}

// Rendering

// Renders the scene on all hardware threads with SSE and keeps the image as PNG for rt_image_data().
// maxTraceDepth limits reflections, as in the CLI.
RT_API int rt_render( RTMcpScene *scene, int width, int height, int maxTraceDepth )
{
    return call( scene, [&]() -> bool
    {
        if ( width < 1 || width > MaxImageSide || height < 1 || height > MaxImageSide )
        {
            return fail( scene, "the image must be 1 to " + std::to_string( MaxImageSide ) + " pixels wide and high" );
        }
        if ( maxTraceDepth < 1 || maxTraceDepth > MaxDepth )
        {
            return fail( scene, "the trace depth must be 1 to " + std::to_string( MaxDepth ) );
        }

        RTRenderSettings settings;
        settings.Width         = width;
        settings.Height        = height;
        settings.MaxTraceDepth = maxTraceDepth;
        const unsigned int hardwareThreads = std::thread::hardware_concurrency();
        settings.NumberOfThreads = hardwareThreads > 0 ? static_cast<int>( hardwareThreads ) : 1;

        RTRenderer renderer( settings );
        std::vector<unsigned char> png = renderer.render( &scene->Scene, scene->Camera ).toPNG();
        if ( png.empty() )
        {
            return fail( scene, "cannot encode the image as PNG" );
        }

        scene->Image.swap( png );
        scene->RenderTime = renderer.getRenderTime();
        return true;
    } );
}

// The PNG file of the last render, rt_image_size() bytes; nullptr before the first render
RT_API const unsigned char* rt_image_data( RTMcpScene *scene )
{
    return scene != nullptr && !scene->Image.empty() ? scene->Image.data() : nullptr;
}

RT_API size_t rt_image_size( RTMcpScene *scene )
{
    return scene != nullptr ? scene->Image.size() : 0;
}

// Seconds the last render took, building the BVH included
RT_API double rt_render_time( RTMcpScene *scene )
{
    return scene != nullptr ? scene->RenderTime : 0.0;
}
