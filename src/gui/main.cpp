#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("High-Precision Calculator"));
    app.setApplicationVersion(QStringLiteral("1.0"));

    MainWindow window;
    window.show();

    return app.exec();
}
