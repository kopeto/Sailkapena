#include "PublicWindow.h"

PublicWindow::PublicWindow() : QWidget()
{
    setWindowTitle("Hitz gurutzatuak");
    resize(300, 200);

    timerlabel = new QLabel();
    timerlabel->setFont(QFont("Arial", 400));
    timerlabel->setTextFormat(Qt::RichText); 
    timerlabel->setAlignment(Qt::AlignCenter);

}

PublicWindow::~PublicWindow()
{
}
