#include "mainwindow.h"
#include <QApplication>
#include <QGLFormat>

int main(int argc, char *argv[])
{
    QGLFormat::setDefaultFormat(QGLFormat::defaultFormat());

    QApplication app(argc, argv);

    MainWindow window;
    window.show();

    return app.exec();
}
