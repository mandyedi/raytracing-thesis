#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QImage>
#include <QMessageBox>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "raytracer/rt_renderer.h"
#include "objects/rt_distant_light.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
#ifdef SSE_ENABLED
    qDebug() << "SSE enabled";
#endif
    // The core loads meshes from files, so copy the embedded ones into a folder that lives as long as the window
    for ( const QString &mesh : QDir( ":/obj" ).entryList( QDir::Files ) )
    {
        QFile::copy( ":/obj/" + mesh, MeshDirectory.filePath( mesh ) );
    }
    Scene.setMeshDirectory( QFile::encodeName( MeshDirectory.path() ).toStdString() );

    CameraRenderer.setCamera( RTVector( -0.8f, 1.6f, 6.0f ), RTVector( 0.0f, 0.0f, 0.0f ), RTCameraTypePerspective, RTCameraViewNone );
    ui->glWidget->init(&Scene, &CameraRenderer, &ManipulatorHandler);
    Scene.setCamera( &CameraRenderer );

    CameraTop.setCamera( RTVector( 0.0f, 3.0f, 0.00001f ), RTVector( 0.0f, 0.0f, 0.0f ), RTCameraTypeOrtho, RTCameraViewTop );
    CameraTop.setScreenSize( 800, 600 );

    CameraFront.setCamera( RTVector( 0.0f, 0.0f, 1.0f ), RTVector( 0.0f, 0.0f, 0.0f ), RTCameraTypeOrtho, RTCameraViewFront );
    CameraFront.setScreenSize( 800, 600 );

    CameraRight.setCamera( RTVector( 1.0f, 0.0f, 0.0f ), RTVector( 0.0f, 0.0f, 0.0f ), RTCameraTypeOrtho, RTCameraViewRight );
    CameraRight.setScreenSize( 800, 600 );

    ImageWidth  = 800;
    ImageHeight = 600;

    QObject::connect( this, SIGNAL( topViewSelected() ), &ManipulatorHandler, SLOT( constraintXZ() ) );
    QObject::connect( this, SIGNAL( frontViewSelected() ), &ManipulatorHandler, SLOT( constraintXY() ) );
    QObject::connect( this, SIGNAL( rightViewSelected() ), &ManipulatorHandler, SLOT( constraintYZ() ) );

    ui->propertiesGroupBox->setEnabled( false );

    ui->imagePartsHComboBox->insertItem( 0, "1" );
    ui->imagePartsHComboBox->insertItem( 1, "2" );
    ui->imagePartsHComboBox->insertItem( 2, "4" );
    ui->imagePartsHComboBox->insertItem( 3, "8" );
    ui->imagePartsHComboBox->insertItem( 4, "16" );
    ui->imagePartsHComboBox->insertItem( 5, "32" );
    ui->imagePartsHComboBox->setCurrentIndex( 5 );

    ui->imagePartsVComboBox->insertItem( 0, "1" );
    ui->imagePartsVComboBox->insertItem( 1, "2" );
    ui->imagePartsVComboBox->insertItem( 2, "4" );
    ui->imagePartsVComboBox->insertItem( 3, "8" );
    ui->imagePartsVComboBox->insertItem( 4, "16" );
    ui->imagePartsVComboBox->insertItem( 5, "32" );
    ui->imagePartsVComboBox->setCurrentIndex( 5 );
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_objectListWidget_itemClicked( QListWidgetItem *item )
{
    Scene.setActiveObject( item->text().toStdString() );
    ManipulatorHandler.setActiveObject( Scene.getActiveObject() );
    updateProperties();
    updateDependentProperties();
}

void MainWindow::on_deselectButton_pressed()
{
    Scene.noActiveObject();
    ManipulatorHandler.noActiveObject();
    updateProperties();
    ui->objectListWidget->selectionModel()->clearSelection();
}

void MainWindow::on_deleteObjectButton_clicked()
{
    if ( Scene.getActiveObject() )
    {
        ui->objectListWidget->takeItem( ui->objectListWidget->row( ui->objectListWidget->currentItem() ) );
        Scene.removeActiveObject();
        Scene.noActiveObject();
        ui->glWidget->updateGL();
    }

    if ( Scene.getActiveLight() )
    {
        ui->objectListWidget->takeItem( ui->objectListWidget->row( ui->objectListWidget->currentItem() ) );
        Scene.removeActiveLight();
        Scene.noActiveLight();
        ui->glWidget->updateGL();
    }
}

void MainWindow::on_actionClear_Scene_triggered()
{
    Scene.noActiveObject();
    Scene.removeAll();
    ui->objectListWidget->clear();
    ui->glWidget->updateGL();
}

void MainWindow::on_setColorButton_clicked()
{
    RTObject *o = Scene.getActiveObject();
    if( o != nullptr )
    {
        QColor color;
        color.setRedF( o->getColor().x() );
        color.setGreenF( o->getColor().y() );
        color.setBlueF( o->getColor().z() );
        color = QColorDialog::getColor( color , this );
        if ( color.isValid() )
        {
            float red = static_cast<float>( color.redF() );
            float green = static_cast<float>( color.greenF() );
            float blue = static_cast<float>( color.blueF() );
            o->setColor( RTVector( red, green, blue ) );
            ui->glWidget->updateGL();
        }
    }
}

void MainWindow::on_renderButton_clicked()
{
    renderImage( false );
}

void MainWindow::on_pushSSEButton_clicked()
{
    renderImage( true );
}

void MainWindow::renderImage( bool useSIMD )
{
    RTRenderSettings settings;
    settings.Width           = ImageWidth;
    settings.Height          = ImageHeight;
    settings.NumberOfThreads = ui->spinBoxNumberOfThreads->value();
    settings.MaxTraceDepth   = ui->maxTraceDepthSpinBox->value();
    settings.UseSIMD         = useSIMD;
    settings.TileWidth       = ImageWidth / ui->imagePartsHComboBox->currentText().toInt();
    settings.TileHeight      = ImageHeight / ui->imagePartsVComboBox->currentText().toInt();

    RTRenderer renderer( settings );
    RTImage image = renderer.render( &Scene, CameraRenderer );
    ui->labelTime->setText( QString( "Rendering time: %1 s" ).arg( renderer.getRenderTime() ) );

    ImageViewerWindow = new ImageViewer;
    ImageViewerWindow->create( image );
    ImageViewerWindow->show();
    ImageViewerWindow->savePNG();
}

void MainWindow::updateProperties()
{
    RTObject *o = Scene.getActiveObject();
    RTLight  *l = Scene.getActiveLight();
    if( o == nullptr && l  == nullptr )
    {
        ui->propertiesGroupBox->setTitle( "no selected object" );
        ui->objectListWidget->selectionModel()->clearSelection();
        ui->propertiesGroupBox->setEnabled( false );
    }
    else if ( o != nullptr )
    {
        ui->propertiesGroupBox->setTitle( QString::fromStdString( o->getName() ) + " properties" );
        ui->SmoothShadingCheckBox->setChecked( o->getSmoothShading() );
        ui->propertiesGroupBox->setEnabled( true );

        ui->diffuseRadioButton->setEnabled( true );
        ui->specularRadioButton->setEnabled( true );
        ui->diffuseAndSpecularRadioButton->setEnabled( true );
        ui->reflectiveRadioButton->setEnabled( true );
        ui->SmoothShadingCheckBox->setEnabled( true );

        ui->distantLightDirectionLabel->setEnabled( false );
        ui->distLightDirXDoubleSpinBox->setEnabled( false );
        ui->distLightDirYDoubleSpinBox->setEnabled( false );
        ui->distLightDirZDoubleSpinBox->setEnabled( false );

        ui->setColorButton->setEnabled( true );

        // Update radio buttons
        if ( o->getMaterialType() == RTObject::RTMaterialType::Diffuse )
        {
            ui->diffuseRadioButton->setChecked( true );
        }
        else if ( o->getMaterialType() == RTObject::RTMaterialType::Specular )
        {
            ui->specularRadioButton->setChecked( true );
        }
        else if ( o->getMaterialType() == RTObject::RTMaterialType::DiffuseAndSpecular )
        {
            ui->diffuseAndSpecularRadioButton->setChecked( true );
        }
        else if ( o->getMaterialType() == RTObject::RTMaterialType::Reflective )
        {
            ui->reflectiveRadioButton->setChecked( true );
        }

    }
    else if ( l != nullptr )
    {
        ui->propertiesGroupBox->setTitle( QString::fromStdString( l->getName() ) + " properties" );

        if ( l->getLightType() == RTLight::RTLightType::Point )
        {
            ui->propertiesGroupBox->setEnabled( false );
        }
        else if ( l->getLightType() == RTLight::RTLightType::Distant )
        {
            ui->propertiesGroupBox->setEnabled( true );

            ui->diffuseRadioButton->setEnabled( false );
            ui->specularRadioButton->setEnabled( false );
            ui->diffuseAndSpecularRadioButton->setEnabled( false );
            ui->reflectiveRadioButton->setEnabled( false );
            ui->SmoothShadingCheckBox->setEnabled( false );

            ui->diffuseCoefficientLabel->setEnabled( false );
            ui->diffuseDoubleSpinBox->setEnabled( false );

            ui->specularCoefficientLabel->setEnabled( false );
            ui->specularLineEdit->setEnabled( false );

            ui->specularExponentLabel->setEnabled( false );
            ui->specularExponentSpinBox->setEnabled( false );

            ui->reflectionLabel->setEnabled( false );
            ui->reflectionDoubleSpinBox->setEnabled( false );

            ui->distantLightDirectionLabel->setEnabled( true );
            ui->distLightDirXDoubleSpinBox->setEnabled( true );
            ui->distLightDirYDoubleSpinBox->setEnabled( true );
            ui->distLightDirZDoubleSpinBox->setEnabled( true );

            ui->setColorButton->setEnabled( false );
        }
    }
}

void MainWindow::updateDependentProperties()
{
    RTObject *o = Scene.getActiveObject();

    if ( !o )
    {
        return;
    }

    if ( o->getMaterialType() == RTObject::RTMaterialType::Diffuse )
    {
        ui->diffuseCoefficientLabel->setEnabled( true );
        ui->diffuseDoubleSpinBox->setEnabled( true );

        ui->specularCoefficientLabel->setEnabled( false );
        ui->specularLineEdit->setEnabled( false );

        ui->specularExponentLabel->setEnabled( false );
        ui->specularExponentSpinBox->setEnabled( false );

        ui->reflectionLabel->setEnabled( false );
        ui->reflectionDoubleSpinBox->setEnabled( false );
    }
    if ( o->getMaterialType() == RTObject::RTMaterialType::Specular )
    {
        ui->diffuseCoefficientLabel->setEnabled( false );
        ui->diffuseDoubleSpinBox->setEnabled( false );

        ui->specularCoefficientLabel->setEnabled( false );
        ui->specularLineEdit->setEnabled( false );

        ui->specularExponentLabel->setEnabled( true );
        ui->specularExponentSpinBox->setEnabled( true );
        ui->specularExponentSpinBox->setValue( static_cast<int>( o->getSpecularExponent() ) );

        ui->reflectionLabel->setEnabled( false );
        ui->reflectionDoubleSpinBox->setEnabled( false );
    }
    else if ( o->getMaterialType() == RTObject::RTMaterialType::DiffuseAndSpecular )
    {
        ui->diffuseCoefficientLabel->setEnabled( true );
        ui->diffuseDoubleSpinBox->setEnabled( true );
        ui->diffuseDoubleSpinBox->setValue( static_cast<double>( o->getDiffuse() ) );

        ui->specularCoefficientLabel->setEnabled( true );
        ui->specularLineEdit->setEnabled( true );

        ui->specularExponentLabel->setEnabled( true );
        ui->specularExponentSpinBox->setEnabled( true );
        ui->specularExponentSpinBox->setValue( static_cast<int>( o->getSpecularExponent() ) );

        ui->reflectionLabel->setEnabled( false );
        ui->reflectionDoubleSpinBox->setEnabled( false );
    }
    else if ( o->getMaterialType() == RTObject::RTMaterialType::Reflective )
    {
        ui->diffuseCoefficientLabel->setEnabled( false );
        ui->diffuseDoubleSpinBox->setEnabled( false );

        ui->specularCoefficientLabel->setEnabled( false );
        ui->specularLineEdit->setEnabled( false );

        ui->specularExponentLabel->setEnabled( false );
        ui->specularExponentSpinBox->setEnabled( false );

        ui->reflectionLabel->setEnabled( true );
        ui->reflectionDoubleSpinBox->setEnabled( true );
        ui->reflectionDoubleSpinBox->setValue( static_cast<double>( o->getReflection() ) );
    }
    else
    {
        ui->diffuseCoefficientLabel->setEnabled( false );
        ui->diffuseDoubleSpinBox->setEnabled( false );

        ui->specularCoefficientLabel->setEnabled( false );
        ui->specularLineEdit->setEnabled( false );

        ui->specularExponentLabel->setEnabled( false );
        ui->specularExponentSpinBox->setEnabled( false );

        ui->reflectionLabel->setEnabled( false );
        ui->reflectionDoubleSpinBox->setEnabled( false );
    }
}

void MainWindow::on_actionSave_Scene_triggered()
{
    QDateTime dateTime = QDateTime::currentDateTime();
    QString fileName = QDir::currentPath() + "/scene_" + dateTime.toString( "yyyyMMdd_hhmmss" ) + ".sc";
    if ( !Scene.saveScene( QFile::encodeName( fileName ).toStdString() ) )
    {
        QMessageBox::warning( this, "Save Scene", "Cannot write " + fileName );
    }
}

void MainWindow::on_actionOpen_Scene_triggered()
{
    QString fileName = QFileDialog::getOpenFileName( this, "Open Scene File", QDir::currentPath(), QString( "Scene (*.sc)" ) );
    if ( !fileName.isEmpty() )
    {
        on_actionClear_Scene_triggered();

        std::string error;
        if ( !Scene.openScene( QFile::encodeName( fileName ).toStdString(), error ) )
        {
            on_actionClear_Scene_triggered();
            QMessageBox::warning( this, "Open Scene File", QString::fromLocal8Bit( error.c_str() ) );
            return;
        }

        for ( RTObject *object : Scene.getObjects() )
        {
            ui->objectListWidget->addItem( QString::fromStdString( object->getName() ) );
        }
        for ( RTLight *light : Scene.getLights() )
        {
            ui->objectListWidget->addItem( QString::fromStdString( light->getName() ) );
        }
        ui->glWidget->updateGL();
    }
}

void MainWindow::selectObjectInList()
{
    QListWidgetItem *item = nullptr;
    for ( int row = 0; row < ui->objectListWidget->count(); row++ )
    {
        item = ui->objectListWidget->item( row );
        if ( Scene.getActiveObject() && item->text() == QString::fromStdString( Scene.getActiveObject()->getName() ) )
        {
            ui->objectListWidget->setCurrentItem(item);
            return;
        }
        else if ( Scene.getActiveLight() && item->text() == QString::fromStdString( Scene.getActiveLight()->getName() ) )
        {
            ui->objectListWidget->setCurrentItem( item );
            return;
        }
    }
}

void MainWindow::on_CameraViewButton_clicked()
{
    ui->MoveCameraGrid->parentWidget()->setEnabled( true );
    ui->glWidget->setCamera( &CameraRenderer );
    ui->glWidget->updateGL();
}

void MainWindow::on_TopViewButton_clicked()
{
    ui->MoveCameraGrid->parentWidget()->setEnabled( false );
    emit topViewSelected();
    ui->glWidget->setCamera( &CameraTop );
    ui->glWidget->updateGL();
}

void MainWindow::on_FrontViewButton_clicked()
{
    ui->MoveCameraGrid->parentWidget()->setEnabled( false );
    emit frontViewSelected();
    ui->glWidget->setCamera( &CameraFront );
    ui->glWidget->updateGL();
}

void MainWindow::on_RightViewButton_clicked()
{
    ui->MoveCameraGrid->parentWidget()->setEnabled( false );
    emit rightViewSelected();
    ui->glWidget->setCamera( &CameraRight );
    ui->glWidget->updateGL();
}

void MainWindow::on_addPlaneButton_clicked()
{
    ui->objectListWidget->addItem( QString::fromStdString( Scene.addPlane() ) );
    ui->glWidget->updateGL();
}

void MainWindow::on_addSphereButton_clicked()
{
    ui->objectListWidget->addItem( QString::fromStdString( Scene.addSphere() ) );
    ui->glWidget->updateGL();
}

void MainWindow::on_addCubeButton_clicked()
{
    ui->objectListWidget->addItem( QString::fromStdString( Scene.addCube() ) );
    ui->glWidget->updateGL();
}

void MainWindow::on_addPyramidButton_clicked()
{
    ui->objectListWidget->addItem( QString::fromStdString( Scene.addPyramid() ) );
    ui->glWidget->updateGL();
}

void MainWindow::on_addPointLightButton_clicked()
{
    ui->objectListWidget->addItem( QString::fromStdString( Scene.addPointLight() ) );
    ui->glWidget->updateGL();
}

void MainWindow::on_addDistantLightButton_clicked()
{
    ui->objectListWidget->addItem( QString::fromStdString( Scene.addDistantLight() ) );
    ui->glWidget->updateGL();
}

void MainWindow::on_addObjButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName( this, "Open Model File", QDir::currentPath(), QString( "Model file (*.obj)" ) );
    if ( !fileName.isEmpty() )
    {
        std::string fileNameString = QFile::encodeName( fileName ).toStdString();

        ui->objectListWidget->addItem( QString::fromStdString( Scene.addObj( fileNameString ) ) );
        ui->glWidget->updateGL();
    }
}

void MainWindow::on_addCylinderButton_clicked()
{
    ui->objectListWidget->addItem( QString::fromStdString( Scene.addCylinder() ) );
    ui->glWidget->updateGL();
}

void MainWindow::on_addConeButton_clicked()
{
    ui->objectListWidget->addItem( QString::fromStdString( Scene.addCone() ) );
    ui->glWidget->updateGL();
}

void MainWindow::on_addTorusButton_clicked()
{
    ui->objectListWidget->addItem( QString::fromStdString( Scene.addTorus() ) );
    ui->glWidget->updateGL();
}

void MainWindow::on_diffuseRadioButton_clicked()
{
    RTObject *o = Scene.getActiveObject();
    if ( o != nullptr )
    {
        o->setMaterialType( RTObject::RTMaterialType::Diffuse );
    }
    updateDependentProperties();
}

void MainWindow::on_specularRadioButton_clicked()
{
    RTObject *o = Scene.getActiveObject();
    if ( o != nullptr )
    {
        o->setMaterialType( RTObject::RTMaterialType::Specular );
    }
    updateDependentProperties();
}

void MainWindow::on_diffuseAndSpecularRadioButton_clicked()
{
    RTObject *o = Scene.getActiveObject();
    if ( o != nullptr )
    {
        o->setMaterialType( RTObject::RTMaterialType::DiffuseAndSpecular );
    }
    updateDependentProperties();
}

void MainWindow::on_reflectiveRadioButton_clicked()
{
    RTObject *o = Scene.getActiveObject();
    if ( o != nullptr )
    {
        o->setMaterialType( RTObject::RTMaterialType::Reflective );
    }
    updateDependentProperties();
}

void MainWindow::on_diffuseDoubleSpinBox_valueChanged( double arg1 )
{
    RTObject *o = Scene.getActiveObject();
    if ( o != nullptr )
    {
        float d = static_cast<float>( arg1 );
        o->setDiffuse( d );
        float s = 1.0f - d;
        o->setSpecular( s );
        ui->specularLineEdit->setText( QString::number( s ) );
    }
}

void MainWindow::on_specularExponentSpinBox_valueChanged( int arg1 )
{
    RTObject *o = Scene.getActiveObject();
    if ( o != nullptr )
    {
        o->setSpecularExponent( static_cast<float>( arg1 ) );
    }
}

void MainWindow::on_reflectionDoubleSpinBox_valueChanged( double arg1 )
{
    RTObject *o = Scene.getActiveObject();
    if ( o != nullptr )
    {
        o->setReflection( static_cast<float>( arg1 ) );
    }
}

void MainWindow::on_SmoothShadingCheckBox_toggled( bool checked )
{
    RTObject *o = Scene.getActiveObject();
    if ( o != nullptr )
    {
        o->setSmoothShading( checked );
    }
}

void MainWindow::on_imagePartsHComboBox_currentIndexChanged( const QString &arg1 )
{
    updateImagePartsLabel();
}

void MainWindow::on_imagePartsVComboBox_currentIndexChanged( const QString &arg1 )
{
    updateImagePartsLabel();
}

void MainWindow::updateImagePartsLabel()
{
    int h = ui->imagePartsHComboBox->currentText().toInt();
    int v = ui->imagePartsVComboBox->currentText().toInt();
    ui->imagePartsLabel->setText( "Image parts: " + QString::number( h * v ) );
}

void MainWindow::on_distLightDirXDoubleSpinBox_valueChanged( double arg1 )
{
    if ( RTDistantLight *l = dynamic_cast<RTDistantLight*>( Scene.getActiveLight() ) )
    {
        l->setDirectionX( static_cast<float>( arg1 ) );
    }
}

void MainWindow::on_distLightDirYDoubleSpinBox_valueChanged( double arg1 )
{
    if ( RTDistantLight *l = dynamic_cast<RTDistantLight*>( Scene.getActiveLight() ) )
    {
        l->setDirectionY( static_cast<float>( arg1 ) );
    }
}

void MainWindow::on_distLightDirZDoubleSpinBox_valueChanged( double arg1 )
{
    if ( RTDistantLight *l = dynamic_cast<RTDistantLight*>( Scene.getActiveLight() ) )
    {
        l->setDirectionZ( static_cast<float>( arg1 ) );
    }
}
