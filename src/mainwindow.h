// mainwindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QApplication>
#include <QMainWindow>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPointer>

#include "Txapelketa.h"
#include "Taula.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow();
    QPointer<Txapelketa> txapelketa;

private:
    // QPointer<QVBoxLayout> mainLayout;

private slots:
    inline void exit()
    {
        QApplication::quit();
    }

    inline void about()
    {
        QMessageBox message(QMessageBox::Information, tr("SailkapenaHG"),
                            tr("Lehiaketako emaitzak eta sailkapena kudeatzeko aplikazioa."),
                            QMessageBox::NoButton, this);
        message.addButton(tr("Ados"), QMessageBox::AcceptRole);
        message.exec();
    }

    inline void taulaGorde()
    {
        txapelketa->saveToFile();
    }

    inline void taulaCsvraEsportatu()
    {
        txapelketa->exportToCsv();
    }

    inline void taulaIreki()
    {
        txapelketa->loadFromFile();
    }

    inline void pantailaPublikoaErakutsi()
    {
        txapelketa->togglePublicWindow();
    }
};

#endif // MAINWINDOW_H
