// mainwindow.h
#ifndef CLASSIFICATION_WINDOW_H
#define CLASSIFICATION_WINDOW_H

#include <QWidget>

#include <stdlib.h>
#include <iostream>

#include <QPointer>

#include "CountDownWidget.h"

class ClassificationWindow : public QWidget {
    Q_OBJECT

public:
    ClassificationWindow(QWidget *parent = nullptr);
    ~ClassificationWindow() = default;

private slots:
    
private:
    QPointer<QVBoxLayout>       mainLayout;
    QPointer<CountdownWidget>   countdown;

};

#endif // CLASSIFICATION_WINDOW_H