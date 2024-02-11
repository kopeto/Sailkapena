// mainwindow.cpp
#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    countdown = new CountdownWidget(2700);

    createInterface();
}

void MainWindow::createInterface()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(countdown);

    setLayout(mainLayout);
}
