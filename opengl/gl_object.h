#ifndef GL_OBJECT_H
#define GL_OBJECT_H

#include <string>
#include <QGLBuffer>
#include <QVector3D>
#include "math/rt_vector.h"

class GLObject
{
public:
    GLObject();
    ~GLObject();
    void release();

    void init( std::string &objFileName );
    void init( const RTVector *vertices, unsigned int numberOfVertices );

    GLenum getDrawMode();

    inline QGLBuffer* getVertexBuffer()
    {
        return &VertexBuffer;
    }

    inline int getNumberOfVertices()
    {
        return NumberOfVertices;
    }

private:
    QGLBuffer   VertexBuffer;
    QVector3D  *Vertices;
    int         NumberOfVertices;

    void createVertices( std::string &objFileName );
    void createVertexBuffer();
};

#endif // GL_OBJECT_H
