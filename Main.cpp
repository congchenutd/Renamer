#include "MainWindow.h"
#include <QApplication>
#include <QDir>

// workaround for a bug on Mavericks
// Finder returns / as the working path of an app bundle
// This method calcluates the path of the bundle from the application's path
QString getCurrentPath()
{
    QDir dir(QApplication::applicationDirPath());
    dir.cdUp();
    dir.cdUp();
    dir.cdUp();
    return dir.absolutePath();
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);

#ifdef Q_OS_OSX
    QDir::setCurrent(getCurrentPath());
#endif

    MainWindow wnd;
    wnd.resize(800, 600);
    wnd.show();

    return app.exec();
}
