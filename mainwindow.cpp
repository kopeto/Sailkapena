// mainwindow.cpp
#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    btnStartTimer = new QPushButton("Start Timer", this);
    countdown = new CountdownWidget(2700);

    createInterface();
}

void MainWindow::createInterface()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(countdown);
    mainLayout->addWidget(btnStartTimer);

    setLayout(mainLayout);
}
