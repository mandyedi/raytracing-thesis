#ifndef GL_OBJECT_H
#define GL_OBJECT_H

#include <QGLBuffer>

class GLObject
{
public:
    GLObject();
    ~GLObject();
    void release();

    void init( std::string &objFileName );

    GLenum getDrawMode();

    inline QGLBuffer* getVertexBuffer()
    {
        return &VertexBuffer;
    }

private:
    QGLBuffer   VertexBuffer;
    QVector3D  *Vertices;
    int         NumberOfVertices;

    void createVertices( std::string &objFileName );
};

#endif // GL_OBJECT_H
