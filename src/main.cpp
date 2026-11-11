#include <QApplication>
#include <QDebug>
#include <QLocale>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QLocale::setDefault(QLocale(QLocale::Basque, QLocale::Spain));
    QApplication a(argc, argv);
    a.setApplicationName("SailkapenaHG");
    a.setApplicationVersion("0.1");

    qInfo() << "Aplikazioaren izena:" << a.applicationName();
    qInfo() << "Aplikazioaren bertsioa:" << a.applicationVersion();
    qInfo() << "Qt bertsioa:" << QT_VERSION_STR;

    MainWindow w;
    w.show();

    return a.exec();
}
