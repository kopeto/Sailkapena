#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    mainLayout = new QVBoxLayout(this);
    setLayout(mainLayout);
}
