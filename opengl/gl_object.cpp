#include <QDebug>
#include <QVector3D>
#include "gl_object.h"
#include "3rd_party/tiny_obj_loader.h"

GLObject::GLObject()
 : VertexBuffer( QGLBuffer::VertexBuffer )
 , Vertices( nullptr )
 , NumberOfVertices( 0 )
{
}

GLObject::~GLObject(){}

void GLObject::release(){}

void GLObject::init( std::string &objFileName )
{
    qDebug() << "GLObj::Init()";

    createVertices( objFileName );

    VertexBuffer.create();
    VertexBuffer.bind();
    VertexBuffer.setUsagePattern( QGLBuffer::StaticDraw );
    VertexBuffer.allocate( Vertices, NumberOfVertices * sizeof( QVector3D ) );

    delete[] Vertices;
    Vertices = nullptr;
}

GLenum GLObject::getDrawMode()
{
    return GL_TRIANGLE_STRIP;
}


void GLObject::createVertices( std::string &objFileName )
{
    std::vector<tinyobj::shape_t> shapes;
    std::string err = tinyobj::LoadObj( shapes, objFileName.c_str() );

    if ( !err.empty() )
    {
        qDebug() << err.c_str() << "\n";
    }
    else
    {
        NumberOfVertices = 0;
        for( unsigned int i = 0; i < shapes.size(); i++ )
        {
            tinyobj::shape_t &shape = shapes[i];
            NumberOfVertices += shape.mesh.indices.size();
        }

        Vertices = new QVector3D[NumberOfVertices];

        unsigned int index = 0;
        for( unsigned int i = 0; i < shapes.size(); i++ )
        {
            tinyobj::shape_t &shape = shapes[i];
            for( unsigned int j = 0; j < shape.mesh.indices.size(); j++ )
            {
                unsigned int positionIndex = shape.mesh.indices[j];
                Vertices[index].setX( shape.mesh.positions[ positionIndex * 3 ] );
                Vertices[index].setY( shape.mesh.positions[ positionIndex * 3 + 1] );
                Vertices[index].setZ( shape.mesh.positions[ positionIndex * 3 + 2] );
                index++;
            }
        }
    }
}
