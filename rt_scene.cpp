#include <stdio.h>
#include <QDebug>
#include <QDateTime>
#include "ui_mainwindow.h"
#include "rt_scene.h"
#include "objects/rt_object.h"
#include "objects/rt_light.h"
#include "objects/rt_point_light.h"
#include "objects/rt_distant_light.h"
#include "opengl/gl_widget.h"
#include "rt_camera.h"

RTScene::RTScene()
 : ActiveObject( nullptr )
 , ActiveObjectIndex( -1 )
 , ObjectNameCount( 0 )
{
}

RTScene::~RTScene()
{
}

void RTScene::addObject( RTObject *object )
{
    Objects.append( object );
}

void RTScene::setActiveObject( QString name )
{
    noActiveObject();
    noActiveLight();

    for ( int i = 0; i < Objects.size(); i++ )
    {
        if( name.compare( Objects[i]->getName() ) == 0 )
        {
            ActiveObject      = Objects[i];
            ActiveObjectIndex = i;
            break;
        }
    }

    for ( int i = 0; i < Lights.size(); i++ )
    {
        if ( name.compare( Lights[i]->getName() ) == 0 )
        {
            ActiveLight      = Lights[i];
            ActiveLightIndex = i;
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

QList<RTObject*> RTScene::getObjects()
{
    return Objects;
}

void RTScene::removeActiveObject()
{
    if ( ActiveObject != nullptr && ActiveObjectIndex != -1 )
    {
        delete Objects.takeAt( ActiveObjectIndex );
    }
}

QString RTScene::addObj( std::string &objFileName )
{
    QString meshName( "Obj File " + QString::number( ObjectNameCount ) );
#ifdef QT_DEBUG
    qDebug() << "DEBUG: Create " << meshName;
#endif

    RTObject *mesh = new RTObject( objFileName, RTVector( 0.0f, 0.0f, 0.0f ) );
    mesh->setObjectType( RTObject::RTObjectType::Obj );
    mesh->setScale( RTVector( 0.5f, 0.5f, 0.5f ) );
    mesh->setName( meshName );
    mesh->setColor( RTVector( 200.0f / 255.0f, 36.0f / 255.0f, 14.0f / 255.0f ) );

    GLObject *glObj = new GLObject;
    glObj->init( objFileName );
    mesh->setGLObject( glObj );

    Objects.append( mesh );
    ObjectNameCount++;

    return meshName;
}

QString RTScene::addSphere()
{
    QString sphereName( "Sphere " + QString::number( ObjectNameCount ) );
#ifdef QT_DEBUG
    qDebug() << "DEBUG: Create " << sphereName;
#endif

    std::string objFile = getResourceFile( "obj", "sphere.obj" );

    RTObject *mesh = new RTObject( objFile, RTVector( 0.0f, 0.0f, 0.0f ) );
    mesh->setObjectType( RTObject::RTObjectType::Sphere );
    mesh->setScale( RTVector( 1.0f, 1.0f, 1.0f ) );
    mesh->setName(sphereName);
    mesh->setColor( RTVector( 0.1569f, 0.6471f, 0.7843f ) );

    GLObject *glObj = new GLObject;
    glObj->init( objFile );
    mesh->setGLObject( glObj );

    Objects.append( mesh );
    ObjectNameCount++;

    return sphereName;
}

QString RTScene::addPlane()
{
    QString meshName( "Plane_" + QString::number( ObjectNameCount ) );
#ifdef QT_DEBUG
    qDebug() << "DEBUG: Create " << meshName;
#endif

    std::string objFile = getResourceFile( "obj", "plane.obj" );

    RTObject *mesh = new RTObject( objFile, RTVector( 0.0f, 0.0f, 0.0f ) );
    mesh->setObjectType( RTObject::RTObjectType::Plane );
    mesh->setScale( RTVector( 3.0f, 3.0f, 3.0f ) );
    mesh->setName( meshName );
    mesh->setColor( RTVector( 0.1f, 0.5f, 0.1f ) );

    GLObject *glObj = new GLObject;
    glObj->init( objFile );
    mesh->setGLObject( glObj );

    Objects.append( mesh );
    ObjectNameCount++;

    return meshName;
}

QString RTScene::addCube()
{
    QString meshName( "Cube_" + QString::number( ObjectNameCount ) );
#ifdef QT_DEBUG
    qDebug() << "DEBUG: Create " << meshName;
#endif

    std::string objFile = getResourceFile( "obj", "cube.obj" );

    RTObject *mesh = new RTObject( objFile, RTVector( -0.5f, 0.0f, -0.5f ) );
    mesh->setObjectType( RTObject::RTObjectType::Cube );
    mesh->setScale( RTVector( 0.5f, 0.5f, 0.5f ) );
    mesh->setName( meshName );
    mesh->setColor( RTVector( 190.0f / 255.0f, 200.0f / 255.0f, 14.0f / 255.0f ) );

    GLObject *glObj = new GLObject;
    glObj->init( objFile );
    mesh->setGLObject( glObj );

    Objects.append( mesh );
    ObjectNameCount++;

    return meshName;
}

QString RTScene::addPyramid()
{
    QString meshName( "Pyramid_" + QString::number( ObjectNameCount ) );
#ifdef QT_DEBUG
    qDebug() << "DEBUG: Create " << meshName;
#endif

    std::string objFile = getResourceFile( "obj", "pyramid.obj" );

    RTObject *mesh = new RTObject( objFile, RTVector( 0.0f, 0.0f, 0.0f ) );
    mesh->setObjectType( RTObject::RTObjectType::Pyramid );
    mesh->setScale( RTVector( 0.5f, 0.5f, 0.5f ) );
    mesh->setName( meshName );
    mesh->setColor( RTVector( 200.0f / 255.0f, 36.0f / 255.0f, 14.0f / 255.0f ) );

    GLObject *glObj = new GLObject;
    glObj->init( objFile );
    mesh->setGLObject( glObj );

    Objects.append( mesh );
    ObjectNameCount++;

    return meshName;
}

QString RTScene::addCylinder()
{
    QString meshName( "Cylinder " + QString::number( ObjectNameCount ) );
#ifdef QT_DEBUG
    qDebug() << "DEBUG: Create " << meshName;
#endif

    std::string objFile = getResourceFile( "obj", "cylinder.obj" );

    RTObject *mesh = new RTObject( objFile, RTVector( 0.0f, 0.0f, 0.0f ) );
    mesh->setObjectType( RTObject::RTObjectType::Cylinder );
    mesh->setScale( RTVector( 0.5f, 0.5f, 0.5f ) );
    mesh->setName( meshName );
    mesh->setColor( RTVector( 200.0f / 255.0f, 36.0f / 255.0f, 14.0f / 255.0f ) );

    GLObject *glObj = new GLObject;
    glObj->init( objFile );
    mesh->setGLObject( glObj );

    Objects.append( mesh );
    ObjectNameCount++;

    return meshName;
}

QString RTScene::addCone()
{
    QString meshName( "Cone " + QString::number( ObjectNameCount ) );
#ifdef QT_DEBUG
    qDebug() << "DEBUG: Create " << meshName;
#endif

    std::string objFile = getResourceFile( "obj", "cone.obj" );

    RTObject *mesh = new RTObject( objFile, RTVector( 0.0f, 0.0f, 0.0f ) );
    mesh->setObjectType( RTObject::RTObjectType::Cone );
    mesh->setScale( RTVector( 0.5f, 0.5f, 0.5f ) );
    mesh->setName( meshName );
    mesh->setColor( RTVector( 200.0f / 255.0f, 36.0f / 255.0f, 14.0f / 255.0f ) );

    GLObject *glObj = new GLObject;
    glObj->init( objFile );
    mesh->setGLObject( glObj );

    Objects.append( mesh );
    ObjectNameCount++;

    return meshName;
}

QString RTScene::addTorus()
{
    QString meshName( "Torus " + QString::number( ObjectNameCount ) );
#ifdef QT_DEBUG
    qDebug() << "DEBUG: Create " << meshName;
#endif

    std::string objFile = getResourceFile( "obj", "torus.obj" );

    RTObject *mesh = new RTObject( objFile, RTVector( 0.0f, 0.0f, 0.0f ) );
    mesh->setObjectType( RTObject::RTObjectType::Tours );
    mesh->setScale( RTVector( 0.5f, 0.5f, 0.5f ) );
    mesh->setName( meshName );
    mesh->setColor( RTVector( 200.0f / 255.0f, 36.0f / 255.0f, 14.0f / 255.0f ) );

    GLObject *glObj = new GLObject;
    glObj->init( objFile );
    mesh->setGLObject( glObj );

    Objects.append( mesh );
    ObjectNameCount++;

    return meshName;
}

void RTScene::setActiveLight( QString name )
{
    noActiveObject();
    for( int i = 0; i < Lights.size(); i++ )
    {
        if( name.compare( Lights[i]->getName() ) == 0)
        {
            ActiveLight      = Lights[i];
            ActiveLightIndex = i;
            break;
        }
    }
}

void RTScene::noActiveLight()
{
    ActiveLight      = nullptr;
    ActiveLightIndex = -1;
}

QString RTScene::addPointLight()
{
    QString lightName( "PointLight_" + QString::number( ObjectNameCount ) );
#ifdef QT_DEBUG
    qDebug() << "DEBUG: Create " << lightName;
#endif
    RTPointLight *light = new RTPointLight( RTVector( 0.0f, 2.0f, 1.0f ), RTVector( 1.0f, 1.0f, 1.0f ), 2.0f );

    std::string objFile = getResourceFile( "obj", "sphere.obj" );
    GLObject *glObj = new GLObject;
    glObj->init( objFile );
    light->setGLObject( glObj );

    light->setName( lightName );
    Lights.append( light );
    ObjectNameCount++;

    return lightName;
}

QString RTScene::addDistantLight()
{
    QString lightName( "DistantLight_" + QString::number( ObjectNameCount ) );
#ifdef QT_DEBUG
    qDebug() << "DEBUG: Create " << lightName;
#endif
    RTDistantLight *light = new RTDistantLight( RTVector( 0.0f, -1.0, 0.0f ), RTVector( 1.0f, 1.0f, 1.0f ), 2.0f );

    std::string objFile = getResourceFile( "obj", "sphere.obj" );
    GLObject *glObj = new GLObject;
    glObj->init( objFile );
    light->setGLObject( glObj );

    light->setName( lightName );
    Lights.append( light );
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

QList<RTLight*> RTScene::getLights()
{
    return Lights;
}

void RTScene::removeActiveLight()
{
    if( ActiveLight != nullptr && ActiveLightIndex != -1)
    {
        delete Lights.takeAt( ActiveLightIndex );
    }
}

void RTScene::setCamera( RTCamera *camera )
{
    Camera = camera;
}

void RTScene::removeAll()
{
    size_t numberOfObjects = Objects.size();
    for( size_t i = 0; i < numberOfObjects; i++ )
    {
        delete Objects.takeAt( 0 );
    }

    size_t numberOfLights = Lights.size();
    for( size_t i = 0; i < numberOfLights; i++ )
    {
        delete Lights.takeAt( 0 );
    }
}

void RTScene::saveScene()
{
    QDateTime dateTime = QDateTime::currentDateTime();
    QFile file(QDir::currentPath() + "/scene_" + dateTime.toString("yyyyMMdd_hhmmss") + ".sc");
    file.open(QIODevice::WriteOnly);
    QTextStream out(&file);

    QVector3D eye = Camera->getEye();
    QVector3D at  = Camera->getAt();
    out << "c " << eye.x() << " " << eye.y() << " " << eye.z() << " " << at.x() << " " << at.y() << " " << at.z() << "\n";

    unsigned int numberOfObjects = Objects.size();
    for( unsigned int i = 0; i < numberOfObjects; i++ )
    {
        RTObject *o = Objects[ i ];

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

    unsigned int numberOfLights = Lights.size();
    for ( size_t i = 0; i < numberOfLights; i++ )
    {
        RTLight *l = Lights[ i ];

        int type = static_cast<int>( l->getLightType() );
        RTVector p = l->getPosition();
        RTVector s = l->getScale();
        RTVector c = l->getColor();
        out << "l " << type << " ";
        out << p.x() << " " << p.y() << " " << p.z() << " ";
        out << s.x() << " " << s.y() << " " << s.z() << " ";
        out << c.x() << " " << c.y() << " " << c.z() << " " << l->getIntensity() << "\n";
    }

    file.close();
}

void RTScene::openScene( FILE *file , Ui::MainWindow *ui )
{
    while( true )
    {
        char header[64];
        if ( fscanf( file, "%s", header ) == EOF )
        {
            break;
        }

        // Camera
        if ( strcmp( header, "c" ) == 0 )
        {
            float ex, ey, ez, ax, ay, az;
            fscanf( file, "%f %f %f %f %f %f\n", &ex, &ey, &ez, &ax, &ay, &az );
            Camera->setCamera( QVector3D( ex, ey, ez ), QVector3D( ax, ay, az ), RTCameraType::RTCameraTypePerspective, RTCameraView::RTCameraViewNone );
        }

        // Object
        if ( strcmp( header, "o" ) == 0 )
        {
            int typeIndex, materialIndex, smooth;
            float px, py, pz, sx, sy, sz, cx, cy, cz;
            float dif, spec, specExp, refl;

            fscanf( file, "%d %f %f %f %f %f %f %f %f %f\n", &typeIndex, &px, &py, &pz, &sx, &sy, &sz, &cx, &cy, &cz );
            fscanf( file, "%d", &materialIndex );
            fscanf( file, "%f %f %f %f %d", &dif, &spec, &specExp, &refl, &smooth );

            RTObject::RTObjectType type = (RTObject::RTObjectType)typeIndex;
            RTObject::RTMaterialType materialType = (RTObject::RTMaterialType)materialIndex;

            QString names[static_cast<unsigned int>( RTObject::RTObjectType::Size )] = { "Plane_", "Sphere_", "Cube_", "Pyramid_", "Cylinder_", "Cone_", "Tours_", "Obj_" };
            QString objFiles[static_cast<unsigned int>( RTObject::RTObjectType::Size )] = { "plane.obj", "sphere.obj", "cube.obj", "pyramid.obj", "cylinder.obj", "cone.obj", "torus.obj" };

            std::string objFile  = getResourceFile( "obj", objFiles[typeIndex] );
            QString     name = names[typeIndex] + QString::number( ObjectNameCount );

            RTObject *mesh = new RTObject( objFile, RTVector( px, py, pz ) );
            mesh->setName( name );
            mesh->setObjectType( type );
            mesh->setMaterialType( materialType );
            mesh->setScale( RTVector( sx, sy, sz ) );
            mesh->setColor( RTVector( cx, cy, cz ) );
            mesh->setDiffuse( dif );
            mesh->setSpecular( spec );
            mesh->setSpecularExponent( specExp );
            mesh->setReflection( refl );
            mesh->setSmoothShading( (bool)smooth );

            GLObject *glObj = new GLObject;
            glObj->init( objFile );
            mesh->setGLObject( glObj );
            Objects.append( mesh );
            ObjectNameCount++;

            ui->objectListWidget->addItem( name );
        }

        // Light
        if ( strcmp( header, "l" ) == 0 )
        {            
            int typeIndex;
            float pdx, pdy, pdz, sx, sy, sz, cx, cy, cz, intensity;

            fscanf( file, "%d %f %f %f %f %f %f %f %f %f %f\n", &typeIndex, &pdx, &pdy, &pdz, &sx, &sy, &sz, &cx, &cy, &cz, &intensity );

            QString names[static_cast<unsigned int>( RTLight::RTLightType::Size )] = { "PointLight_", "DistantLight_" };

            std::string objFile  = getResourceFile( "obj", "sphere.obj" );
            QString     name = names[typeIndex] + QString::number( ObjectNameCount );

            RTPointLight *light = new RTPointLight( RTVector( pdx, pdy, pdz ), RTVector( cx, cy, cz ), intensity );
            light->setName( name );

            GLObject *glObj = new GLObject;
            glObj->init( objFile );
            light->setGLObject( glObj );
            Lights.append( light );
            ObjectNameCount++;

            ui->objectListWidget->addItem( name );
        }
    }
}

/*static*/ std::string RTScene::getResourceFile( const QString &prefix, const QString &resource )
{
    QTemporaryDir temporaryDir;
    QFile::copy( ":/" + prefix + "/" + resource, temporaryDir.path() + "/" + resource );
    return QString( temporaryDir.path() + "/" + resource ).toLatin1().data();
}
