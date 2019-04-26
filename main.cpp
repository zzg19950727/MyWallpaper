#include <QApplication>
#include "desktopgo.h"
#include <QtSerialPort/QSerialPort>
#include <QWindow>
#include <QTimer>
int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    DesktopGo go;
    return app.exec();
}
