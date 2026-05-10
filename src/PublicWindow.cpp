#include "PublicWindow.h"

PublicWindow::PublicWindow() : QWidget()
{
    setWindowTitle("SailkapenaHG");
    resize(300, 200);

    timerlabel = new QLabel();
    {
        QFont f("Arial");
        f.setStyleHint(QFont::SansSerif); // fallback to system sans-serif on Linux
        f.setPointSize(400);
        timerlabel->setFont(f);
    }
    timerlabel->setTextFormat(Qt::RichText); 
    timerlabel->setAlignment(Qt::AlignCenter);

}

PublicWindow::~PublicWindow()
{
}
