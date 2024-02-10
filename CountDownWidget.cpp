#include "CountDownWidget.h"

CountdownWidget::CountdownWidget(int countdown_seconds) : QWidget()
{
    countdown_seconds_ = countdown_seconds;
    remaining_seconds_ = countdown_seconds_;

    timer_ = new QTimer(this);
    connect(timer_, &QTimer::timeout, this, &CountdownWidget::updateCountdown);

    input_field_ = new QLineEdit();
    input_field_->setPlaceholderText("Enter number of seconds");

    start_button_ = new QPushButton("Iniciar");
    connect(start_button_, &QPushButton::clicked, this, &CountdownWidget::startCountdown);

    label_ = new QLabel();
    updateCountdown();

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(label_);
    layout->addWidget(input_field_);
    layout->addWidget(start_button_);
    setLayout(layout);
}

void CountdownWidget::setInitialValue(int secs)
{
    countdown_seconds_ = secs;
}

void CountdownWidget::updateCountdown()
{
    remaining_seconds_ -= 1;
    if (remaining_seconds_ <= 0)
    {
        timer_->stop();
        label_->setText("Denbora bukatu da.");
    }
    else
    {
        int hours = remaining_seconds_ / (60 * 60); // 60*60
        int mins = (remaining_seconds_ / 60) % 60;
        int secs = remaining_seconds_ % 60;
        label_->setText(QString("%1%2:%3").arg((hours > 0 ? QString("%1:").arg(hours) : "")).arg(mins, 2, 10, QChar('0')).arg(secs, 2, 10, QChar('0')));
    }
}

void CountdownWidget::startCountdown()
{
    QString inputText = input_field_->text();
    bool ok;
    countdown_seconds_ = inputText.toInt(&ok);
    if (ok)
    {
        remaining_seconds_ = countdown_seconds_;
        updateCountdown();
        timer_->start(1000); // Initial delay of 1 second
    }
    else
    {
        label_->setText("Por favor ingrese un número válido.");
    }
}
