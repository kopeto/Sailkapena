#include "ClassificationWindow.h"

ClassificationWindow::ClassificationWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("Sailkapena");
    countdown = new CountdownWidget(2700);
    mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(countdown);

    setLayout(mainLayout);
}
