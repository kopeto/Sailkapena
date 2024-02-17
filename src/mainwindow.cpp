#include "mainwindow.h"

MainWindow::MainWindow()
    : QMainWindow()
{
    mainLayout  = new QVBoxLayout(this);
    txapelketa  = new Txapelketa(this);
    
    /* Layouts */
    mainLayout->addWidget(txapelketa);

    setCentralWidget(txapelketa);
}
