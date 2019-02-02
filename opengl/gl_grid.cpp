#include <QDebug>

#include "gl_grid.h"

GLGrid::GLGrid()
{

}

GLGrid::~GLGrid()
{

}

void GLGrid::init()
{
    createVertexData();
    createVertexVBO();
}

QGLBuffer *GLGrid::getVertexBuffer()
{
    return VBOVertex;
}

int GLGrid::getNumberOfVertices()
{
    return NumberOfVertices;
}

GLenum GLGrid::getDrawMode()
{
    return GL_LINES;
}

void GLGrid::destroyVBO()
{
    VBOVertex->destroy();
}

void GLGrid::createVertexData()
{
    float min = -2.0f, max = 2.0f;
    unsigned int n = 20;
    float step = ( max - min ) / static_cast<float>( n );
    float y = 0.0f;
    NumberOfVertices = 100;
    Vertices = new QVector3D[NumberOfVertices];

    unsigned int count = 0;
    for ( unsigned int x = 0; x <= n; x++ )
    {
        float xx = static_cast<float>( x ) * step + min;
        Vertices[count] = QVector3D(xx, y, min);
        count++;
        Vertices[count] = QVector3D(xx, y, max);
        count++;
    }

    for( unsigned int z = 0; z <= n; z++ )
    {
        float zz = static_cast<float>( z ) * step + min;
        Vertices[count] = QVector3D( min, y, zz );
        count++;
        Vertices[count] = QVector3D( max, y, zz );
        count++;
    }

    qDebug() << "m_NumberOfVertices: " << NumberOfVertices;
    qDebug() << "count" << count;
}

void GLGrid::createVertexVBO()
{
    VBOVertex = new QGLBuffer( QGLBuffer::VertexBuffer );
    VBOVertex->create();
    VBOVertex->bind();
    VBOVertex->setUsagePattern( QGLBuffer::StaticDraw );
    VBOVertex->allocate( Vertices, NumberOfVertices * sizeof( QVector3D ) );
    delete Vertices;
}
