#ifndef CPU_INFO_WINDOW_H
#define CPU_INFO_WINDOW_H

#include <QMainWindow>

namespace Ui {
class CpuInfoWindow;
}

class CpuInfoWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit CpuInfoWindow(QWidget *parent = 0);
    ~CpuInfoWindow();

private:
    QString boolToStringConverter(bool _b);

    Ui::CpuInfoWindow *ui;
};

#endif // CPU_INFO_WINDOW_H
