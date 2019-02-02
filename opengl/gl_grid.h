#ifndef GRID_H
#define GRID_H

#include <QVector3D>
#include <QGLBuffer>

class GLGrid
{
public:
    GLGrid();
    ~GLGrid();

    void       init();
    QGLBuffer* getVertexBuffer();
    int        getNumberOfVertices();
    GLenum     getDrawMode();
    void       destroyVBO();

private:
    void createVertexData();
    void createVertexVBO();

    int        NumberOfVertices;
    QVector3D *Vertices;
    QGLBuffer *VBOVertex;
};

#endif // GRID_H
