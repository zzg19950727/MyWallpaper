#include <QApplication>
#include "desktopgo.h"

#include <QWindow>
int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    DesktopGo go;
    return app.exec();
}
