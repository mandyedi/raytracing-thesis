#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QGLWidget>
#include <QtOpenGL>
#include <QMatrix4x4>

#include "gl_grid.h"
#include "../rt_camera.h"
#include "../rt_scene.h"
#include "manipulators/manipulator.h"

class GLWidget : public QGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    GLWidget(QWidget *parent = 0);
    virtual ~GLWidget();

    void init(RTScene *scene, RTCamera *camera, Manipulator *manipulator);

    inline void setCamera( RTCamera *camera )
    {
        Camera = camera;
    }

public slots:
    void moveCameraForward();
    void moveCameraBackward();
    void moveCameraRight();
    void moveCameraLeft();
    void moveCameraUp();
    void moveCameraDown();
    void setMoveStep(double moveStep);

signals:
    void objectSelected();

protected:
    void initializeGL();
    void resizeGL( int width, int height );
    void paintGL();
    void mousePressEvent( QMouseEvent *event );
    void mouseMoveEvent( QMouseEvent *event );

private:
    void createShaders();

    QGLShaderProgram *ShaderProgram;
    QMatrix4x4        ProjectionMatrix;
    QMatrix4x4        ModelViewMatrix;

    RTCamera    *Camera;
    RTScene     *Scene;
    Manipulator *ManipulatorHandler;
    GLGrid       Grid;

    float MoveStep;
    int   LastMousePosX;
    int   LastMousePosY;

    RTVector HitPoint;
    RTVector MovePoint;
};

#endif // GLWIDGET_H
