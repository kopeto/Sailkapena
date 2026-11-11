#include "mainwindow.h"

MainWindow::MainWindow()
    : QMainWindow()
{
    setWindowTitle(tr("SailkapenaHG"));
    txapelketa = new Txapelketa(this);
    setCentralWidget(txapelketa);
    showMaximized();

    QMenu *fileMenu = menuBar()->addMenu(tr("&Fitxategia"));
    QMenu *viewMenu = menuBar()->addMenu(tr("&Ikusi"));
    QMenu *helpMenu = menuBar()->addMenu(tr("&Laguntza"));

    QAction *saveAction = new QAction(tr("&Emaitzak gorde"), this);
    connect(saveAction, &QAction::triggered, this, &MainWindow::taulaGorde);
    QAction *exportAction = new QAction(tr("CSVra &esportatu"), this);
    connect(exportAction, &QAction::triggered, this, &MainWindow::taulaCsvraEsportatu);
    QAction *openAction = new QAction(tr("Emaitzak &ireki"), this);
    connect(openAction, &QAction::triggered, this, &MainWindow::taulaIreki);
    QAction *formatAction = new QAction(tr("Txapelketaren formatua editatu (&JSON)"), this);
    connect(formatAction, &QAction::triggered, txapelketa, &Txapelketa::editTournamentFormat);
    QAction *exitAction = new QAction(tr("&Irten"), this);
    connect(exitAction, &QAction::triggered, this, &MainWindow::exit);

    fileMenu->addAction(saveAction);
    fileMenu->addAction(exportAction);
    fileMenu->addAction(openAction);
    fileMenu->addAction(formatAction);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);

    QAction *publicWindowAction = new QAction(tr("Pantaila publikoa erakutsi/ezkutatu"), this);
    connect(publicWindowAction, &QAction::triggered, this, &MainWindow::pantailaPublikoaErakutsi);
    viewMenu->addAction(publicWindowAction);

    QAction *aboutAction = new QAction(tr("&Aplikazioari buruz"), this);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::about);
    helpMenu->addAction(aboutAction);
}
