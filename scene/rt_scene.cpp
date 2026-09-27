#include <fstream>
#include <iostream>
#include <sstream>
#include "rt_scene.h"
#include "objects/rt_object.h"
#include "objects/rt_light.h"
#include "objects/rt_point_light.h"
#include "objects/rt_distant_light.h"
#include "rt_camera.h"

RTScene::RTScene()
 : Camera( nullptr )
 , ActiveObject( nullptr )
 , ActiveObjectIndex( -1 )
 , ActiveLight( nullptr )
 , ActiveLightIndex( -1 )
 , ObjectNameCount( 0 )
{
}

RTScene::~RTScene()
{
    removeAll();
}

void RTScene::addObject( RTObject *object )
{
    Objects.push_back( object );
}

void RTScene::setActiveObject( const std::string &name )
{
    noActiveObject();
    noActiveLight();

    for ( size_t i = 0; i < Objects.size(); i++ )
    {
        if ( name == Objects[i]->getName() )
        {
            ActiveObject      = Objects[i];
            ActiveObjectIndex = static_cast<int>( i );
            break;
        }
    }

    for ( size_t i = 0; i < Lights.size(); i++ )
    {
        if ( name == Lights[i]->getName() )
        {
            ActiveLight      = Lights[i];
            ActiveLightIndex = static_cast<int>( i );
            break;
        }
    }
}

void RTScene::noActiveObject()
{
    ActiveObject      = nullptr;
    ActiveObjectIndex = -1;
}

size_t RTScene::getNumberOfObjects()
{
    return Objects.size();
}

RTObject *RTScene::getObject(const int &i)
{
    return Objects[i];
}

RTObject* RTScene::getActiveObject()
{
    return ActiveObject;
}

const std::vector<RTObject*>& RTScene::getObjects()
{
    return Objects;
}

void RTScene::removeActiveObject()
{
    if ( ActiveObject != nullptr && ActiveObjectIndex != -1 )
    {
        delete Objects[ActiveObjectIndex];
        Objects.erase( Objects.begin() + ActiveObjectIndex );
    }
}

std::string RTScene::addMesh( RTObject *mesh, const std::string &name )
{
    if ( !mesh->isLoaded() )
    {
        std::cerr << "Cannot load mesh: " << mesh->getLoadError() << std::endl;
    }

    mesh->setName( name );
    Objects.push_back( mesh );
    ObjectNameCount++;

    return name;
}

std::string RTScene::addObj( const std::string &objFileName )
{
    RTObject *mesh = new RTObject( objFileName, RTVector( 0.0f, 0.0f, 0.0f ) );
    mesh->setObjectType( RTObject::RTObjectType::Obj );
    mesh->setScale( RTVector( 0.5f, 0.5f, 0.5f ) );
    mesh->setColor( RTVector( 200.0f / 255.0f, 36.0f / 255.0f, 14.0f / 255.0f ) );

    return addMesh( mesh, "Obj File " + std::to_string( ObjectNameCount ) );
}

std::string RTScene::addSphere()
{
    RTObject *mesh = new RTObject( getMeshFile( "sphere.obj" ), RTVector( 0.0f, 0.0f, 0.0f ) );
    mesh->setObjectType( RTObject::RTObjectType::Sphere );
    mesh->setScale( RTVector( 1.0f, 1.0f, 1.0f ) );
    mesh->setColor( RTVector( 0.1569f, 0.6471f, 0.7843f ) );

    return addMesh( mesh, "Sphere " + std::to_string( ObjectNameCount ) );
}

std::string RTScene::addPlane()
{
    RTObject *mesh = new RTObject( getMeshFile( "plane.obj" ), RTVector( 0.0f, 0.0f, 0.0f ) );
    mesh->setObjectType( RTObject::RTObjectType::Plane );
    mesh->setScale( RTVector( 3.0f, 3.0f, 3.0f ) );
    mesh->setColor( RTVector( 0.1f, 0.5f, 0.1f ) );

    return addMesh( mesh, "Plane_" + std::to_string( ObjectNameCount ) );
}

std::string RTScene::addCube()
{
    RTObject *mesh = new RTObject( getMeshFile( "cube.obj" ), RTVector( -0.5f, 0.0f, -0.5f ) );
    mesh->setObjectType( RTObject::RTObjectType::Cube );
    mesh->setScale( RTVector( 0.5f, 0.5f, 0.5f ) );
    mesh->setColor( RTVector( 190.0f / 255.0f, 200.0f / 255.0f, 14.0f / 255.0f ) );

    return addMesh( mesh, "Cube_" + std::to_string( ObjectNameCount ) );
}

std::string RTScene::addPyramid()
{
    RTObject *mesh = new RTObject( getMeshFile( "pyramid.obj" ), RTVector( 0.0f, 0.0f, 0.0f ) );
    mesh->setObjectType( RTObject::RTObjectType::Pyramid );
    mesh->setScale( RTVector( 0.5f, 0.5f, 0.5f ) );
    mesh->setColor( RTVector( 200.0f / 255.0f, 36.0f / 255.0f, 14.0f / 255.0f ) );

    return addMesh( mesh, "Pyramid_" + std::to_string( ObjectNameCount ) );
}

std::string RTScene::addCylinder()
{
    RTObject *mesh = new RTObject( getMeshFile( "cylinder.obj" ), RTVector( 0.0f, 0.0f, 0.0f ) );
    mesh->setObjectType( RTObject::RTObjectType::Cylinder );
    mesh->setScale( RTVector( 0.5f, 0.5f, 0.5f ) );
    mesh->setColor( RTVector( 200.0f / 255.0f, 36.0f / 255.0f, 14.0f / 255.0f ) );

    return addMesh( mesh, "Cylinder " + std::to_string( ObjectNameCount ) );
}

std::string RTScene::addCone()
{
    RTObject *mesh = new RTObject( getMeshFile( "cone.obj" ), RTVector( 0.0f, 0.0f, 0.0f ) );
    mesh->setObjectType( RTObject::RTObjectType::Cone );
    mesh->setScale( RTVector( 0.5f, 0.5f, 0.5f ) );
    mesh->setColor( RTVector( 200.0f / 255.0f, 36.0f / 255.0f, 14.0f / 255.0f ) );

    return addMesh( mesh, "Cone " + std::to_string( ObjectNameCount ) );
}

std::string RTScene::addTorus()
{
    RTObject *mesh = new RTObject( getMeshFile( "torus.obj" ), RTVector( 0.0f, 0.0f, 0.0f ) );
    mesh->setObjectType( RTObject::RTObjectType::Tours );
    mesh->setScale( RTVector( 0.5f, 0.5f, 0.5f ) );
    mesh->setColor( RTVector( 200.0f / 255.0f, 36.0f / 255.0f, 14.0f / 255.0f ) );

    return addMesh( mesh, "Torus " + std::to_string( ObjectNameCount ) );
}

void RTScene::setActiveLight( const std::string &name )
{
    noActiveObject();
    for ( size_t i = 0; i < Lights.size(); i++ )
    {
        if ( name == Lights[i]->getName() )
        {
            ActiveLight      = Lights[i];
            ActiveLightIndex = static_cast<int>( i );
            break;
        }
    }
}

void RTScene::noActiveLight()
{
    ActiveLight      = nullptr;
    ActiveLightIndex = -1;
}

std::string RTScene::addPointLight()
{
    std::string lightName( "PointLight_" + std::to_string( ObjectNameCount ) );
    RTPointLight *light = new RTPointLight( RTVector( 0.0f, 2.0f, 1.0f ), RTVector( 1.0f, 1.0f, 1.0f ), 2.0f );

    light->setName( lightName );
    Lights.push_back( light );
    ObjectNameCount++;

    return lightName;
}

std::string RTScene::addDistantLight()
{
    std::string lightName( "DistantLight_" + std::to_string( ObjectNameCount ) );
    RTDistantLight *light = new RTDistantLight( RTVector( 0.0f, -1.0, 0.0f ), RTVector( 1.0f, 1.0f, 1.0f ), 2.0f );

    light->setName( lightName );
    Lights.push_back( light );
    ObjectNameCount++;

    return lightName;
}

size_t RTScene::getNumberOfLights()
{
    return Lights.size();
}

RTLight* RTScene::getLight( const int i )
{
    return Lights[ i ];
}

RTLight* RTScene::getActiveLight()
{
    return ActiveLight;
}

const std::vector<RTLight*>& RTScene::getLights()
{
    return Lights;
}

void RTScene::removeActiveLight()
{
    if( ActiveLight != nullptr && ActiveLightIndex != -1)
    {
        delete Lights[ActiveLightIndex];
        Lights.erase( Lights.begin() + ActiveLightIndex );
    }
}

void RTScene::setCamera( RTCamera *camera )
{
    Camera = camera;
}

void RTScene::removeAll()
{
    for ( RTObject *object : Objects )
    {
        delete object;
    }
    Objects.clear();

    for ( RTLight *light : Lights )
    {
        delete light;
    }
    Lights.clear();

    noActiveObject();
    noActiveLight();
}

void RTScene::setMeshDirectory( const std::string &directory )
{
    MeshDirectory = directory;
}

std::string RTScene::getMeshFile( const std::string &fileName )
{
    return MeshDirectory.empty() ? fileName : MeshDirectory + "/" + fileName;
}

bool RTScene::saveScene( const std::string &fileName )
{
    // Binary mode: "\n" line endings on every platform
    std::ofstream out( fileName.c_str(), std::ios::binary );
    if ( !out )
    {
        return false;
    }

    RTVector eye = Camera->getEye();
    RTVector at  = Camera->getAt();
    out << "c " << eye.x() << " " << eye.y() << " " << eye.z() << " " << at.x() << " " << at.y() << " " << at.z() << "\n";

    for ( RTObject *o : Objects )
    {
        int type = static_cast<int>( o->getObjectType() );
        RTVector p = o->getPosition();
        RTVector s = o->getScale();
        RTVector c = o->getColor();
        out << "o " << type << " ";
        out << p.x() << " " << p.y() << " " << p.z() << " ";
        out << s.x() << " " << s.y() << " " << s.z() << " ";
        out << c.x() << " " << c.y() << " " << c.z() << " ";
        out << static_cast<int>( o->getMaterialType() ) << " ";
        out << o->getDiffuse() << " " << o->getSpecular() << " " << o->getSpecularExponent() << " ";
        out << o->getReflection() << " " << static_cast<int>( o->getSmoothShading() ) << "\n";
    }

    for ( RTLight *l : Lights )
    {
        int type = static_cast<int>( l->getLightType() );

        // A distant light has no position; its direction is stored in that field
        RTVector p = l->getPosition();
        if ( RTDistantLight *distantLight = dynamic_cast<RTDistantLight*>( l ) )
        {
            p = distantLight->getDirection();
        }

        RTVector s = l->getScale();
        RTVector c = l->getColor();
        out << "l " << type << " ";
        out << p.x() << " " << p.y() << " " << p.z() << " ";
        out << s.x() << " " << s.y() << " " << s.z() << " ";
        out << c.x() << " " << c.y() << " " << c.z() << " " << l->getIntensity() << "\n";
    }

    out.close();
    return !out.fail();
}

bool RTScene::openScene( const std::string &fileName, std::string &error )
{
    std::ifstream file( fileName.c_str() );
    if ( !file )
    {
        error = "Cannot open scene file " + fileName;
        return false;
    }

    std::string line;
    int lineNumber = 0;
    while ( std::getline( file, line ) )
    {
        lineNumber++;

        std::istringstream in( line );
        std::string header;
        if ( !( in >> header ) )
        {
            continue;
        }

        std::string lineError;
        if ( header == "c" )
        {
            lineError = readCamera( in );
        }
        else if ( header == "o" )
        {
            lineError = readObject( in );
        }
        else if ( header == "l" )
        {
            lineError = readLight( in );
        }
        else
        {
            lineError = "unknown line type '" + header + "'";
        }

        if ( !lineError.empty() )
        {
            error = fileName + ":" + std::to_string( lineNumber ) + ": " + lineError;
            return false;
        }
    }

    return true;
}

std::string RTScene::readCamera( std::istream &in )
{
    float ex, ey, ez, ax, ay, az;
    if ( !( in >> ex >> ey >> ez >> ax >> ay >> az ) )
    {
        return "a camera line needs 6 numbers: eye.xyz at.xyz";
    }

    if ( Camera != nullptr )
    {
        Camera->setCamera( RTVector( ex, ey, ez ), RTVector( ax, ay, az ), RTCameraType::RTCameraTypePerspective, RTCameraView::RTCameraViewNone );
    }

    return "";
}

std::string RTScene::readObject( std::istream &in )
{
    int typeIndex, materialIndex, smooth;
    float px, py, pz, sx, sy, sz, cx, cy, cz;
    float dif, spec, specExp, refl;
    if ( !( in >> typeIndex >> px >> py >> pz >> sx >> sy >> sz >> cx >> cy >> cz
               >> materialIndex >> dif >> spec >> specExp >> refl >> smooth ) )
    {
        return "an object line needs 16 values: type pos.xyz scale.xyz color.rgb material diffuse specular specExp reflection smooth";
    }

    if ( typeIndex == static_cast<int>( RTObject::RTObjectType::Obj ) )
    {
        return "custom .obj objects (type 7) can't be loaded, the scene file doesn't store their file path";
    }
    if ( typeIndex < 0 || typeIndex > static_cast<int>( RTObject::RTObjectType::Obj ) )
    {
        return "unknown object type " + std::to_string( typeIndex );
    }
    if ( materialIndex < 0 || materialIndex >= static_cast<int>( RTObject::RTMaterialType::Size ) )
    {
        return "unknown material " + std::to_string( materialIndex );
    }

    static const char *names[]    = { "Plane_", "Sphere_", "Cube_", "Pyramid_", "Cylinder_", "Cone_", "Tours_" };
    static const char *objFiles[] = { "plane.obj", "sphere.obj", "cube.obj", "pyramid.obj", "cylinder.obj", "cone.obj", "torus.obj" };

    RTObject *mesh = new RTObject( getMeshFile( objFiles[typeIndex] ), RTVector( px, py, pz ) );
    if ( !mesh->isLoaded() )
    {
        std::string loadError = mesh->getLoadError();
        delete mesh;
        return "cannot load mesh: " + loadError;
    }

    mesh->setName( names[typeIndex] + std::to_string( ObjectNameCount ) );
    mesh->setObjectType( static_cast<RTObject::RTObjectType>( typeIndex ) );
    mesh->setMaterialType( static_cast<RTObject::RTMaterialType>( materialIndex ) );
    mesh->setScale( RTVector( sx, sy, sz ) );
    mesh->setColor( RTVector( cx, cy, cz ) );
    mesh->setDiffuse( dif );
    mesh->setSpecular( spec );
    mesh->setSpecularExponent( specExp );
    mesh->setReflection( refl );
    mesh->setSmoothShading( smooth != 0 );

    Objects.push_back( mesh );
    ObjectNameCount++;

    return "";
}

std::string RTScene::readLight( std::istream &in )
{
    int typeIndex;
    float px, py, pz, sx, sy, sz, cx, cy, cz, intensity;
    if ( !( in >> typeIndex >> px >> py >> pz >> sx >> sy >> sz >> cx >> cy >> cz >> intensity ) )
    {
        return "a light line needs 11 values: type pos.xyz scale.xyz color.rgb intensity";
    }

    RTLight *light = nullptr;
    std::string name;
    if ( typeIndex == static_cast<int>( RTLight::RTLightType::Point ) )
    {
        light = new RTPointLight( RTVector( px, py, pz ), RTVector( cx, cy, cz ), intensity );
        name  = "PointLight_";
    }
    else if ( typeIndex == static_cast<int>( RTLight::RTLightType::Distant ) )
    {
        // The position field holds the direction of a distant light. Keep it as saved, not normalized:
        // the GUI's direction boxes don't normalize it either, so the reloaded scene renders the same.
        RTDistantLight *distantLight = new RTDistantLight( RTVector( px, py, pz ), RTVector( cx, cy, cz ), intensity );
        distantLight->setDirectionX( px );
        distantLight->setDirectionY( py );
        distantLight->setDirectionZ( pz );
        light = distantLight;
        name  = "DistantLight_";
    }
    else
    {
        return "unknown light type " + std::to_string( typeIndex );
    }

    light->setName( name + std::to_string( ObjectNameCount ) );
    Lights.push_back( light );
    ObjectNameCount++;

    return "";
}
