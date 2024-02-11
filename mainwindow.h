// mainwindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QTimer>

#include <stdlib.h>
#include <iostream>

#include "CountDownWidget.h"

class MainWindow : public QWidget {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    
private:
    CountdownWidget *countdown;

    void createInterface();

};

#endif // MAINWINDOW_H
