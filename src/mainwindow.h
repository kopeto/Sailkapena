// mainwindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QTimer>

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
    QPointer<Txapelketa>    txapelketa;

private:
    QPointer<QVBoxLayout>   mainLayout;
};

#endif // MAINWINDOW_H
