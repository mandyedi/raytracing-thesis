#include <QVector3D>
#include "gl_widget.h"
#include "gl_object.h"
#include "objects/rt_object.h"
#include "objects/rt_light.h"

static QVector3D toQVector3D( const RTVector &v )
{
    return QVector3D( v.x(), v.y(), v.z() );
}

GLWidget::GLWidget( QWidget *parent )
    : QGLWidget( parent )
{
}

GLWidget::~GLWidget(){}

void GLWidget::init( RTScene *scene, RTCamera *camera, Manipulator *manipulator )
{
    Scene  = scene;
    Camera = camera;
    ManipulatorHandler  = manipulator;
}

void GLWidget::initializeGL()
{
    qDebug() << "GLWidget::initializeGL()";

    initializeOpenGLFunctions();

    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LESS );

    MoveStep = 0.2;
    createShaders();

    Grid.init();
}

void GLWidget::resizeGL( int width, int height )
{
    Camera->setScreenSize( width, height );
    glViewport( 0, 0, width, height );
    updateGL();
}

void GLWidget::paintGL()
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    ShaderProgram->bind();
    updateProjectionMatrix();
    ShaderProgram->setUniformValue( "projectionMatrix", ProjectionMatrix );

    createMissingGLObjects();

    // Draw objects from scene
    for ( RTObject *object : Scene->getObjects() )
    {
        ModelViewMatrix.setToIdentity();
        ModelViewMatrix.lookAt( toQVector3D( Camera->getEye() ), toQVector3D( Camera->getAt() ), toQVector3D( Camera->getUp() ) );
        ModelViewMatrix.translate({
                                        object->getPosition().x(),
                                        object->getPosition().y(),
                                        object->getPosition().z()
                                    });
        ModelViewMatrix.scale({
                                    object->getScale().x(),
                                    object->getScale().y(),
                                    object->getScale().z()
                                });
        ShaderProgram->setUniformValue( "modelViewMatrix", ModelViewMatrix );

        QVector3D color({
                            object->getColor().x(),
                            object->getColor().y(),
                            object->getColor().z()
                        });
        ShaderProgram->setUniformValue( "color", color );

        object->getGlObject()->getVertexBuffer()->bind();
        ShaderProgram->enableAttributeArray( "vPosition" );
        ShaderProgram->setAttributeBuffer( "vPosition", GL_FLOAT, 0, 3, 0 );

        glDrawArrays( object->getGlObject()->getDrawMode(), 0, object->getGlObject()->getNumberOfVertices() );

        object->getGlObject()->getVertexBuffer()->release();
    }

    // Draw lights from scene
    for ( RTLight *light : Scene->getLights() )
    {
        ModelViewMatrix.setToIdentity();
        ModelViewMatrix.lookAt( toQVector3D( Camera->getEye() ), toQVector3D( Camera->getAt() ), toQVector3D( Camera->getUp() ) );
        ModelViewMatrix.translate({
                                        light->getPosition().x(),
                                        light->getPosition().y(),
                                        light->getPosition().z()
                                    });
        ModelViewMatrix.scale({
                                    light->getScale().x(),
                                    light->getScale().y(),
                                    light->getScale().z()
                                });
        ShaderProgram->setUniformValue( "modelViewMatrix", ModelViewMatrix );

        QVector3D color( 1.0f, 1.0f, 1.0f );
        ShaderProgram->setUniformValue( "color", color );

        light->getGlObject()->getVertexBuffer()->bind();
        ShaderProgram->enableAttributeArray( "vPosition" );
        ShaderProgram->setAttributeBuffer( "vPosition", GL_FLOAT, 0, 3, 0 );

        glDrawArrays( light->getGlObject()->getDrawMode(), 0, light->getGlObject()->getNumberOfVertices() );

        light->getGlObject()->getVertexBuffer()->release();
    }

    // Draw grid
    {
        ModelViewMatrix.setToIdentity();
        ModelViewMatrix.lookAt( toQVector3D( Camera->getEye() ), toQVector3D( Camera->getAt() ), toQVector3D( Camera->getUp() ) );
        ShaderProgram->setUniformValue( "modelViewMatrix", ModelViewMatrix );

        Grid.getVertexBuffer()->bind();
        ShaderProgram->enableAttributeArray( "vPosition" );
        ShaderProgram->setAttributeBuffer( "vPosition", GL_FLOAT, 0, 3, 0 );

        ShaderProgram->setUniformValue( "color", QVector3D( 0.5f, 0.5f, 0.5f ) );

        glDrawArrays(Grid.getDrawMode(), 0, Grid.getNumberOfVertices());

        Grid.getVertexBuffer()->release();
    }

    ShaderProgram->release();
}

void GLWidget::moveCameraForward()
{
    Camera->moveCamera( RTVector( 0.0f, 0.0f, -MoveStep ) );
    updateGL();

}

void GLWidget::moveCameraBackward()
{
    Camera->moveCamera( RTVector(0.0f, 0.0f, MoveStep ) );
    updateGL();
}

void GLWidget::moveCameraRight()
{
    Camera->moveCamera( RTVector( MoveStep, 0.0f, 0.0f ) );
    updateGL();
}

void GLWidget::moveCameraLeft()
{
    Camera->moveCamera( RTVector( -MoveStep, 0.0f, 0.0f ) );
    updateGL();
}

void GLWidget::moveCameraUp()
{
    Camera->moveCamera( RTVector( 0.0f, MoveStep, 0.0f ) );
    updateGL();
}

void GLWidget::moveCameraDown()
{
    Camera->moveCamera( RTVector( 0.0f, -MoveStep, 0.0f ) );
    updateGL();
}

void GLWidget::setMoveStep( double moveStep )
{
    MoveStep = float( moveStep );
}

void GLWidget::createShaders()
{
    QGLShader *vertexShader = new QGLShader( QGLShader::Vertex );
    if( !vertexShader->compileSourceFile( ":/shaders/vertex.glsl" ) )
    {
        qWarning() << vertexShader->log();
    }

    QGLShader *fragmentShader = new QGLShader( QGLShader::Fragment );
    if( !fragmentShader->compileSourceFile( ":/shaders/fragment.glsl" ) )
    {
        qWarning() << fragmentShader->log();
    }

    ShaderProgram = new QGLShaderProgram;
    ShaderProgram->addShader( vertexShader );
    ShaderProgram->addShader( fragmentShader );

    if ( !ShaderProgram->link() )
    {
        qWarning() << ShaderProgram->log() << endl;
    }

    delete vertexShader;
    delete fragmentShader;

}

void GLWidget::updateProjectionMatrix()
{
    ProjectionMatrix.setToIdentity();

    if ( Camera->getCameraType() == RTCameraTypePerspective )
    {
        ProjectionMatrix.perspective(
                    45.0f,
                    static_cast<qreal>( Camera->getScreenWidth() ) / static_cast<qreal>( Camera->getScreenHeight() ),
                    0.1f,
                    20.0f );
    }
    else if ( Camera->getCameraType() == RTCameraTypeOrtho )
    {
        float aspectRatio = Camera->getAspectRatio();
        float zoom        = Camera->getZoom();
        ProjectionMatrix.ortho( -aspectRatio * zoom, aspectRatio * zoom, -zoom, zoom, 0.1, 10 );
    }
}

// Scene objects and lights get their vertex buffers here, where the GL context is current
void GLWidget::createMissingGLObjects()
{
    for ( RTObject *object : Scene->getObjects() )
    {
        if ( object->getGlObject() == nullptr )
        {
            std::shared_ptr<GLObject> glObject = std::make_shared<GLObject>();
            glObject->init( object->getVertices(), object->getNumberOfVertices() );
            object->setGLObject( glObject );
        }
    }

    for ( RTLight *light : Scene->getLights() )
    {
        if ( light->getGlObject() == nullptr )
        {
            std::string sphereFile = Scene->getMeshFile( "sphere.obj" );
            std::shared_ptr<GLObject> glObject = std::make_shared<GLObject>();
            glObject->init( sphereFile );
            light->setGLObject( glObject );
        }
    }
}

void GLWidget::mousePressEvent( QMouseEvent *event )
{
    setFocus();

    LastMousePosX = event->x();
    LastMousePosY = event->y();

    if ( Camera->getCameraView() != RTCameraViewNone )
    {
        RTRay ray;
        Camera->setRayDirection( event->y(), event->x(), ray );

        RTObject     *object   = nullptr;
        RTLight      *light    = nullptr;
        float         distance = 1000000.0f;
        unsigned int  triangleIndex;
        float         u, v;

        for( size_t i = 0; i < Scene->getNumberOfObjects(); i++ )
        {
            // The object may have moved since the last render
            Scene->getObject( i )->updateWorldSpace();
            if( Scene->getObject( i )->intersect( ray, distance, triangleIndex, u, v, true ) )
            {
                LastMousePosX = event->x();
                object = Scene->getObject( i );
                Scene->setActiveObject( object->getName() );
                ManipulatorHandler->setActiveObject( object );
                emit objectSelected();
                qDebug() << object->getName().c_str() << " selected";
            }
        }

        for( size_t i = 0; i < Scene->getNumberOfLights(); i++ )
        {
            if( Scene->getLight( i )->intersect( ray, distance ) )
            {
                object = nullptr;
                LastMousePosX = event->x();
                light = Scene->getLight( i );
                Scene->setActiveLight( light->getName() );
                ManipulatorHandler->setActiveLight( light );
                emit objectSelected();
                qDebug() << light->getName().c_str() << " selected";
            }
        }

        if( object == nullptr )
        {
            Scene->noActiveObject();
            ManipulatorHandler->noActiveObject();
            emit objectSelected();
        }

        if ( light == nullptr ) {
           Scene->noActiveLight();
           ManipulatorHandler->noActiveLight();
           emit objectSelected();
        }

        if ( object == nullptr && light == nullptr )
        {
            return;
        }

        RTVector hitPoint  = ray.Origin + ray.Direction * distance;
        HitPoint  = hitPoint;
        MovePoint = hitPoint;
        ManipulatorHandler->initialize( { hitPoint.x(), hitPoint.y(), hitPoint.z() } );

        updateGL();
    }
}

void GLWidget::mouseMoveEvent( QMouseEvent *event )
{
    if ( Camera->getCameraView() == RTCameraViewNone )
    {
        float differenceX = static_cast<float>( ( event->x() - LastMousePosX ) ) / -100.0f;
        float differenceY = static_cast<float>( ( event->y() - LastMousePosY ) ) / -100.0f;

        Camera->rotateCamera( differenceX, differenceY );

        LastMousePosX = event->x();
        LastMousePosY = event->y();
    }
    else
    {
        if( Scene->getActiveObject() || Scene->getActiveLight() )
        {
            RTRay ray;
            Camera->setRayDirection( event->y(), event->x(), ray );

            ManipulatorHandler->update( ray );
            LastMousePosX = event->x();
        }
    }

    updateGL();
}
