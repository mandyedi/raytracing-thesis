#ifndef IMAGE_VIEWER_H
#define IMAGE_VIEWER_H

#include <QMainWindow>
#include <QLabel>
#include <QScrollArea>
#include "math/rt_vector.h"

class ImageViewer : public QMainWindow
{
    Q_OBJECT
public:
    explicit ImageViewer( QWidget *parent = 0 );

    void create( RTVector **buffer, int imageWidth, int imageHeight );
    void savePNG();
    void open( QImage *_image );

private:
    QLabel      *m_ImageLabel;
    QImage      *m_Image;

};

#endif // IMAGE_VIEWER_H
