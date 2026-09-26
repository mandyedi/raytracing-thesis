#include <QDir>
#include <QMessageBox>
#include <QDebug>
#include <QDateTime>
#include "image_viewer.h"

ImageViewer::ImageViewer( QWidget *parent )
    : QMainWindow( parent )
{
    m_ImageLabel = new QLabel;
    m_ImageLabel->setBackgroundRole( QPalette::Base );
    m_ImageLabel->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );
    m_ImageLabel->setScaledContents( true );

    setCentralWidget( m_ImageLabel );
}

void ImageViewer::create( const RTImage &image )
{
    const int imageWidth  = image.getWidth();
    const int imageHeight = image.getHeight();
    const std::vector<unsigned char> rgb = image.toRGB8();

    m_Image = new QImage( imageWidth, imageHeight, QImage::Format_RGB32 );
    for ( int i = 0; i < imageHeight; ++i )
    {
        for ( int j = 0; j < imageWidth; ++j )
        {
            const unsigned char *pixel = &rgb[( i * imageWidth + j ) * 3];
            m_Image->setPixel( j, i, qRgb( pixel[0], pixel[1], pixel[2] ) );
        }
    }

    m_ImageLabel->setPixmap( QPixmap::fromImage( *m_Image ) );
    m_ImageLabel->adjustSize();
    resize( m_Image->width(), m_Image->height() );
}

void ImageViewer::savePNG()
{
    QDateTime dateTime = QDateTime::currentDateTime();
    m_Image->save( QDir::currentPath() + "/savedFromProg_" + dateTime.toString( "yyyyMMdd_hhmmss" ) + ".png", "PNG" );
}

void ImageViewer::open( QImage *_image )
{
    m_Image = _image;
    if( m_Image->isNull() )
    {
        qDebug() << "In ImageViwer::open: Cannot open rendered image.";
    }
    else
    {
        m_ImageLabel->setPixmap( QPixmap::fromImage( *m_Image ) );
        m_ImageLabel->adjustSize();
        resize( m_Image->width(), m_Image->height() );
    }
}
