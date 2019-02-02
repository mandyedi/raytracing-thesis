#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QTime>

#include "rt_scene.h"
#include "rt_camera.h"
#include "raytracer/rt_tracer.h"
#include "image_viewer.h"
#include "manipulators/manipulator.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    void topViewSelected();
    void frontViewSelected();
    void rightViewSelected();

private slots:

    void on_objectListWidget_itemClicked(QListWidgetItem *item);

    void on_deselectButton_pressed();

    void on_setColorButton_clicked();

    void on_renderButton_clicked();

    void on_pushSSEButton_clicked();

    void updateProperties();

    void updateDependentProperties();

    void on_deleteObjectButton_clicked();

    void on_actionClear_Scene_triggered();

    void on_actionSave_Scene_triggered();

    void on_actionOpen_Scene_triggered();

    void refreshLabelTime();

    void selectObjectInList();

    void on_CameraViewButton_clicked();

    void on_TopViewButton_clicked();

    void on_FrontViewButton_clicked();

    void on_RightViewButton_clicked();

    void on_addPlaneButton_clicked();

    void on_addSphereButton_clicked();

    void on_addCubeButton_clicked();

    void on_addPyramidButton_clicked();

    void on_addPointLightButton_clicked();

    void on_addDistantLightButton_clicked();

    void on_addObjButton_clicked();

    void on_addCylinderButton_clicked();

    void on_addConeButton_clicked();

    void on_addTorusButton_clicked();

    void on_diffuseRadioButton_clicked();

    void on_specularRadioButton_clicked();

    void on_diffuseAndSpecularRadioButton_clicked();

    void on_reflectiveRadioButton_clicked();

    void on_diffuseDoubleSpinBox_valueChanged(double arg1);

    void on_specularExponentSpinBox_valueChanged(int arg1);

    void on_reflectionDoubleSpinBox_valueChanged(double arg1);

    void on_SmoothShadingCheckBox_toggled(bool checked);

    void on_imagePartsHComboBox_currentIndexChanged(const QString &arg1);

    void on_imagePartsVComboBox_currentIndexChanged(const QString &arg1);

    void updateImagePartsLabel();

    void on_distLightDirXDoubleSpinBox_valueChanged(double arg1);

    void on_distLightDirYDoubleSpinBox_valueChanged(double arg1);

    void on_distLightDirZDoubleSpinBox_valueChanged(double arg1);

private:
    Ui::MainWindow *ui;

    RTScene     Scene;
    RTCamera    CameraRenderer;
    RTCamera    CameraTop;
    RTCamera    CameraFront;
    RTCamera    CameraRight;
    Manipulator ManipulatorHandler;
    QTime       Timer;

    int    ImageWidth;
    int    ImageHeight;

    ImageViewer   *ImageViewerWindow;
};

#endif // MAINWINDOW_H
