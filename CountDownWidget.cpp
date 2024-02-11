#include "CountDownWidget.h"

static void setLabel(QLabel *label, int seconds)
{
    int hours = seconds / (60 * 60); // 60*60
    int mins = (seconds / 60) % 60;
    int secs = seconds % 60;
    label->setText(QString("%1%2:%3").arg((hours > 0 ? QString("%1:").arg(hours) : "")).arg(mins, 2, 10, QChar('0')).arg(secs, 2, 10, QChar('0')));
}

CountdownWidget::CountdownWidget(int countdown_seconds) : QWidget()
{
    countdown_seconds_ = countdown_seconds;
    remaining_seconds_ = countdown_seconds_;

    timer_ = new QTimer(this);

    input_field_ = new QLineEdit("5000");
    input_field_->setMaximumWidth(100);
    input_field_->setPlaceholderText("5000");

    start_BTN = new QPushButton("Hasi");
    setInitialValue_BTN = new QPushButton("Set Initial Timer value");

    label_ = new QLabel();
    label_->setFont(QFont("Arial", 200));
    updateCountdown();

    main_layout = new QVBoxLayout(this);
    layout_h = new QHBoxLayout(this);
    main_layout->addWidget(label_);
    main_layout->addLayout(layout_h);
    layout_h->addWidget(input_field_);
    layout_h->addWidget(setInitialValue_BTN);
    main_layout->addWidget(start_BTN);

    connect(timer_, &QTimer::timeout, this, &CountdownWidget::updateCountdown);
    connect(start_BTN, &QPushButton::clicked, this, &CountdownWidget::startCountdown);
    connect(setInitialValue_BTN, &QPushButton::clicked, this, std::bind(&CountdownWidget::setInitialValue, this, input_field_));
}

void CountdownWidget::setInitialValue(QLineEdit *input)
{
    int secs = input->text().toInt();
    qDebug() << "Set initial value: " << secs;
    countdown_seconds_ = secs;
    setLabel(label_, secs);
    timer_->stop();
}

void CountdownWidget::updateCountdown()
{
    remaining_seconds_ -= 1;
    if (remaining_seconds_ <= 0)
    {
        static QPalette pal = label_->palette();
        pal.setColor(QPalette::Window, QColor(Qt::red));

        label_->setAutoFillBackground(true); // IMPORTANT!
        label_->setPalette(pal);

        timer_->stop();

        remaining_seconds_ = countdown_seconds_;
        label_->setText("Denbora bukatu da.");
    }
    else
    {
        int hours = remaining_seconds_ / (60 * 60); // 60*60
        int mins = (remaining_seconds_ / 60) % 60;
        int secs = remaining_seconds_ % 60;
        setLabel(label_, remaining_seconds_);
        // label_->setText(QString("%1%2:%3").arg((hours > 0 ? QString("%1:").arg(hours) : "")).arg(mins, 2, 10, QChar('0')).arg(secs, 2, 10, QChar('0')));
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
        label_->setText("Insert a valid number.");
    }
}
