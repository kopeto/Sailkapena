#include "CountDownWidget.h"

#include "TimeFormat.h"
#include "setLabel.h"

#include <QAbstractButton>
#include <QFont>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include <algorithm>

namespace
{
void showInformation(QWidget *parent, const QString &title, const QString &text)
{
    QMessageBox message(QMessageBox::Information, title, text, QMessageBox::NoButton, parent);
    message.addButton(QObject::tr("Ados"), QMessageBox::AcceptRole);
    message.exec();
}
}

CountdownWidget::CountdownWidget(QWidget *parent)
    : QWidget(parent)
{
    timer_ = new QTimer(this);
    timer_->setInterval(100);

    inputField_ = new QLineEdit(this);
    inputField_->setMaximumWidth(100);
    inputField_->setPlaceholderText(tr("Minutuak"));
    inputField_->setValidator(new QIntValidator(1, 1440, inputField_));

    startButton_ = new QPushButton(tr("Hasi"), this);
    startButton_->setMaximumWidth(150);
    setInitialValueButton_ = new QPushButton(tr("Ezarri denbora"), this);
    setInitialValueButton_->setMaximumWidth(150);
    hideShowToggleButton_ = new QPushButton(tr("Erakutsi/Ezkutatu"), this);
    hideShowToggleButton_->setMaximumWidth(200);

    timerLabel_ = new QLabel(this);
    QFont font("Arial");
    font.setStyleHint(QFont::SansSerif);
    font.setPointSize(150);
    timerLabel_->setFont(font);
    timerLabel_->setTextFormat(Qt::RichText);
    timerLabel_->setAlignment(Qt::AlignCenter);
    setLabel(timerLabel_, 0);

    mainLayout_ = new QVBoxLayout(this);
    horizontalLayout_ = new QHBoxLayout();
    horizontalLayout_->setAlignment(Qt::AlignLeft);
    mainLayout_->addWidget(timerLabel_);
    mainLayout_->addLayout(horizontalLayout_);
    horizontalLayout_->addWidget(inputField_);
    horizontalLayout_->addWidget(setInitialValueButton_);
    horizontalLayout_->addWidget(startButton_);
    horizontalLayout_->addWidget(hideShowToggleButton_);

    connect(timer_, &QTimer::timeout, this, &CountdownWidget::updateCountdown);
    connect(startButton_, &QPushButton::clicked, this, &CountdownWidget::startCountdown);
    connect(setInitialValueButton_, &QPushButton::clicked, this, &CountdownWidget::setInitialValue);
    connect(hideShowToggleButton_, &QPushButton::clicked, this, &CountdownWidget::toggleHideShow);
}

void CountdownWidget::setInitialValue()
{
    bool valid = false;
    const int minutes = inputField_->text().toInt(&valid);
    if (!valid || minutes < 1 || minutes > 1440)
    {
        showInformation(this, tr("Kronometroa"), tr("Sartu 1 eta 1440 arteko minutu kopurua."));
        return;
    }

    timer_->stop();
    elapsedClock_.invalidate();
    hasStarted_ = false;
    countdownSeconds_ = minutes * 60;
    remainingSeconds_ = countdownSeconds_;
    setBlankBackground(timerLabel_);
    setLabel(timerLabel_, remainingSeconds_);
    if (public_label)
        setLabel(public_label, remainingSeconds_);
}

void CountdownWidget::updateCountdown()
{
    if (!hasStarted_ || !elapsedClock_.isValid())
        return;

    const qint64 elapsedMilliseconds = elapsedClock_.elapsed();
    const int elapsedSeconds = static_cast<int>(elapsedMilliseconds / 1000);
    remainingSeconds_ = std::max(0, countdownSeconds_ - elapsedSeconds);
    setBlankBackground(timerLabel_);
    setLabel(timerLabel_, remainingSeconds_);
    if (public_label)
        setLabel(public_label, remainingSeconds_);

    if (elapsedMilliseconds >= static_cast<qint64>(countdownSeconds_) * 1000)
    {
        remainingSeconds_ = 0;
        setLabel(timerLabel_, 0);
        if (public_label)
            setLabel(public_label, 0);
        setRedBackground(timerLabel_);
        timer_->stop();
    }
}

void CountdownWidget::startCountdown()
{
    if (countdownSeconds_ <= 0)
    {
        showInformation(this, tr("Kronometroa"), tr("Ezarri denbora hasi aurretik."));
        return;
    }

    remainingSeconds_ = countdownSeconds_;
    hasStarted_ = true;
    elapsedClock_.start();
    setBlankBackground(timerLabel_);
    setLabel(timerLabel_, remainingSeconds_);
    if (public_label)
        setLabel(public_label, remainingSeconds_);
    timer_->start();
}

void CountdownWidget::toggleHideShow()
{
    if (timerLabel_->isVisible())
        timerLabel_->hide();
    else
        timerLabel_->show();
}

int CountdownWidget::getElapsedTime() const
{
    if (!hasStarted_ || !elapsedClock_.isValid())
        return 0;
    const qint64 elapsedSeconds = elapsedClock_.elapsed() / 1000;
    return static_cast<int>(std::clamp<qint64>(elapsedSeconds, 0, countdownSeconds_));
}

bool CountdownWidget::hasStarted() const
{
    return hasStarted_;
}

void CountdownWidget::setPublicLabel(QLabel *label)
{
    public_label = label;
    if (public_label)
    {
        setLabel(public_label, remainingSeconds_);
        if (hasStarted_ && remainingSeconds_ == 0)
            setRedBackground(public_label);
        else
            setBlankBackground(public_label);
    }
}
