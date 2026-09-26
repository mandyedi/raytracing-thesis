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

void ImageViewer::create( RTVector **buffer, int imageWidth, int imageHeight )
{
    m_Image = new QImage( imageWidth, imageHeight, QImage::Format_RGB32 );
    int count = 0;
    for ( int i = 0; i < imageHeight; ++i )
    {
        for ( int j = 0; j < imageWidth; ++j )
        {
            int r = (uchar)( std::min( 1.0f, buffer[i][j].x() ) * 255 );
            int g = (uchar)( std::min( 1.0f, buffer[i][j].y() ) * 255 );
            int b = (uchar)( std::min( 1.0f, buffer[i][j].z() ) * 255 );
            m_Image->setPixel( j, i, qRgb( r, g, b ) );
            count++;
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
