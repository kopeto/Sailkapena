#include "mainwindow.h"

MainWindow::MainWindow()
    : QMainWindow()
{
    // mainLayout = new QVBoxLayout(this);
    txapelketa = new Txapelketa(this);

    /* Layouts */
    // mainLayout->->addWidget(txapelketa);
    this->layout()->addWidget(txapelketa);
    setCentralWidget(txapelketa);


    /**
     * 
     * MENU
     *  
     */
    QMenu *fileMenu = menuBar()->addMenu(tr("&Fitxategia"));
    QMenu *helpMenu = menuBar()->addMenu(tr("&Laguntza"));

    QAction *exitAction = new QAction(tr("&Irten"), this);
    connect(exitAction, &QAction::triggered, this, &MainWindow::exit);

    QAction *taulaGorde = new QAction(tr("&Taula Gorde"), this);
    connect(taulaGorde, &QAction::triggered, this, &MainWindow::taulaGorde);

    QAction *taulaIreki = new QAction(tr("&Taula Ireki"), this);
    connect(taulaIreki, &QAction::triggered, this, &MainWindow::taulaIreki);

    fileMenu->addAction(exitAction);
    fileMenu->addAction(taulaGorde);
    fileMenu->addAction(taulaIreki);

    QAction *aboutAction = new QAction(tr("&Ze behar duk?"), this);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::about);
    helpMenu->addAction(aboutAction);

    /*********************/
}
