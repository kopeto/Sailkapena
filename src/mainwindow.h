// mainwindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QTimer>

#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>

#include <stdlib.h>
#include <iostream>

#include <QPointer>
#include <QPushButton>
#include <QLineEdit>
#include <QHBoxLayout>

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
        // QMessageBox::StandardButton reply;
        // reply = QMessageBox::question(this, "Irten", "Seguru al hago atera nahi dukela?", QMessageBox::Yes | QMessageBox::No);
        // if (reply == QMessageBox::Yes)
        QApplication::quit();
    }

    inline void about()
    {
        QMessageBox::about(this, "Hitz Gurutzatuen Txapelketa", "Zer jakin nahi duk?");
    }

    inline void taulaGorde()
    {
        txapelketa->saveToFile();
    }

    inline void taulaExceleratu()
    {
        txapelketa->saveToExcel();
    }

    inline void taulaIreki()
    {
        txapelketa->loadFromFile();
    }

    
};

#endif // MAINWINDOW_H
