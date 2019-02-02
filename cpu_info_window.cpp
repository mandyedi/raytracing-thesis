#include <QThread>

#include "cpu_info_window.h"
#include "ui_cpu_info_window.h"

#include "3rd_party/cpuid/cpuid.h"

using namespace Vc;

CpuInfoWindow::CpuInfoWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CpuInfoWindow)
{
    ui->setupUi(this);

    ui->mmxLabel->setText("mmx: " + boolToStringConverter(Vc::CpuId::hasMmx()));
    ui->sseLabel->setText("sse: " + boolToStringConverter(Vc::CpuId::hasSse()));
    ui->sse2Label->setText("sse2: " + boolToStringConverter(Vc::CpuId::hasSse2()));
    ui->sse3Label->setText("sse3: " + boolToStringConverter(Vc::CpuId::hasSse3()));
    ui->ssse3Label->setText("ssse3: " + boolToStringConverter(Vc::CpuId::hasSsse3()));
    ui->sse41Label->setText("sse41: " + boolToStringConverter(Vc::CpuId::hasSse41()));
    ui->sse42Label->setText("sse42: " + boolToStringConverter(Vc::CpuId::hasSse42()));
    ui->sse4aLabel->setText("sse4a: " + boolToStringConverter(Vc::CpuId::hasSse4a()));
    ui->threadQtLabel->setText(QString("ideal thread (QThread): %1").arg(QThread::idealThreadCount()));
    ui->threadCpuIdLabel->setText(QString("logical proc. (cpuId): %1").arg(Vc::CpuId::logicalProcessors()));
}

CpuInfoWindow::~CpuInfoWindow()
{
    delete ui;
}

QString CpuInfoWindow::boolToStringConverter(bool _b)
{
    return _b ? "yes" : "no";
}
