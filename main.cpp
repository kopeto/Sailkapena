#include <QApplication>
#include "mainwindow.h"
#include "ClassificationWindow.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    a.setApplicationName("HitzGurutzatuak");
    a.setApplicationVersion("0.1");

    qInfo() << "Application name: " << a.applicationName();
    qInfo() << "Application version: " << a.applicationVersion();

    qInfo() << "Qt version: " << QT_VERSION_STR;

    ClassificationWindow cw;
    MainWindow w;
    w.show();
    cw.show();

    return a.exec();
}