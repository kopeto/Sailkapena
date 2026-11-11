#include "PublicWindow.h"

#include <QFont>
#include <QVBoxLayout>

PublicWindow::PublicWindow(QWidget *parent)
    : QWidget(parent, Qt::Window), timerLabel_(new QLabel(this)), layout_(new QVBoxLayout(this))
{
    setWindowTitle(tr("Pantaila publikoa - SailkapenaHG"));
    resize(800, 360);

    QFont font("Arial");
    font.setStyleHint(QFont::SansSerif);
    font.setPointSize(150);
    timerLabel_->setFont(font);
    timerLabel_->setAlignment(Qt::AlignCenter);
    timerLabel_->setTextFormat(Qt::RichText);
    timerLabel_->setText("00:00");
    layout_->addWidget(timerLabel_);
}

QLabel *PublicWindow::timerLabel() const
{
    return timerLabel_;
}
