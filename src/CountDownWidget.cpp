#include "CountDownWidget.h"

#include "setLabel.h"

CountdownWidget::CountdownWidget(QWidget* parent) : QWidget(parent)
{
    timer_ = new QTimer(this);

    /* Inputs */
    input_field_ = new QLineEdit("");
    input_field_->setMaximumWidth(100);
    input_field_->setPlaceholderText("");
    /* Buttons */
    start_BTN = new QPushButton("Hasi");
    start_BTN->setMaximumWidth(150);
    setInitialValue_BTN = new QPushButton("Denbora berritu");
    setInitialValue_BTN->setMaximumWidth(150);
    hideShowToggle_BTN = new QPushButton("Azaldu/Ezkutatu");
    hideShowToggle_BTN->setMaximumWidth(200);

    /* Labels */
    timer_Label = new QLabel();
    timer_Label->setFont(QFont("Arial", 400));
    timer_Label->setTextFormat(Qt::RichText); 
    timer_Label->setAlignment(Qt::AlignCenter);
    setLabel(timer_Label, 0);

    /* Layouts */
    main_layout = new QVBoxLayout(this);
    layout_h = new QHBoxLayout(this);
    layout_h->setAlignment(Qt::AlignLeft);
    main_layout->addWidget(timer_Label);
    main_layout->addLayout(layout_h);
    layout_h->addWidget(input_field_);
    layout_h->addWidget(setInitialValue_BTN);
    layout_h->addWidget(start_BTN);
    layout_h->addWidget(hideShowToggle_BTN);

    /* Connections */
    connect(timer_, &QTimer::timeout, this, &CountdownWidget::updateCountdown);
    connect(start_BTN, &QPushButton::clicked, this, &CountdownWidget::startCountdown);
    connect(setInitialValue_BTN, &QPushButton::clicked, this, std::bind(&CountdownWidget::setInitialValue, this, input_field_));
    connect(hideShowToggle_BTN, &QPushButton::clicked, this, &CountdownWidget::toggleHideShow);
}

void CountdownWidget::setInitialValue(QLineEdit *input)
{
    setBlankBackground(timer_Label);
    bool ok;
    int secs = input->text().toInt(&ok);
    if (!ok)
    {
        countdown_seconds_ = 0;
        setLabel(timer_Label, 0);
        if(public_label)
            setLabel(public_label, 0);
        timer_->stop();
        return;
    }

    countdown_seconds_ = secs;
    setLabel(timer_Label, secs);
    if(public_label)
            setLabel(public_label, secs);
    timer_->stop();
}

void CountdownWidget::updateCountdown()
{
    remaining_seconds_ -= 1;
    if (remaining_seconds_ < 0)
    {
        setRedBackground(timer_Label);
        timer_->stop();

        // timer_Label->setText("00:00");
    }
    else
    {
        setBlankBackground(timer_Label);
        int hours = remaining_seconds_ / (60 * 60); // 60*60
        int mins = (remaining_seconds_ / 60) % 60;
        int secs = remaining_seconds_ % 60;
        setLabel(timer_Label, remaining_seconds_);
        setLabel(public_label, remaining_seconds_);
    }
}

void CountdownWidget::startCountdown()
{
    remaining_seconds_ = countdown_seconds_;
    updateCountdown();
    timer_->start(1000); // Initial delay of 1 second
}

void CountdownWidget::toggleHideShow()
{
    if(timer_Label->isVisible())
    {
        timer_Label->hide();
    }
    else
    {
        timer_Label->show();
    }
}


int CountdownWidget::getElapsedTime()
{
    return countdown_seconds_ - remaining_seconds_;
}