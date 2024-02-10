// mainwindow.cpp
#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent) {
    lblTimeRemaining =  new QLabel("Time remaining: 0", this);
    btnStartTimer =     new QPushButton("Start Timer", this);
    maintimer =         new QTimer();
    createInterface();

}

void MainWindow::createInterface() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(lblTimeRemaining);
    mainLayout->addWidget(btnStartTimer);

    setLayout(mainLayout);
}


