// mainwindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QTimer>

#include <stdlib.h>

class MainWindow : public QWidget {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private slots:

private:
    QTimer          *maintimer;
    QLabel          *lblTimeRemaining;
    QPushButton     *btnStartTimer;

    void createInterface();
    void startTimer();
};

#endif // MAINWINDOW_H
