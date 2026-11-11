#include "PublicWindow.h"
#include "Taula.h"

#include <QFont>
#include <QSizePolicy>
#include <QVBoxLayout>

PublicWindow::PublicWindow(QWidget *parent)
    : QWidget(parent, Qt::Window), timerLabel_(new QLabel(this)), standingsTable_(new Taula(this)),
      layout_(new QVBoxLayout(this))
{
    setWindowTitle(tr("Pantaila publikoa - SailkapenaHG"));
    resize(1200, 760);

    QFont font("Arial");
    font.setStyleHint(QFont::SansSerif);
    font.setPointSize(150);
    timerLabel_->setFont(font);
    timerLabel_->setAlignment(Qt::AlignCenter);
    timerLabel_->setTextFormat(Qt::RichText);
    timerLabel_->setText("00:00");
    timerLabel_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    standingsTable_->setPublicDisplay(true);
    standingsTable_->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    standingsTable_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    layout_->addWidget(timerLabel_);
    layout_->addWidget(standingsTable_, 1);
}

QLabel *PublicWindow::timerLabel() const
{
    return timerLabel_;
}

Taula *PublicWindow::standingsTable() const
{
    return standingsTable_;
}
