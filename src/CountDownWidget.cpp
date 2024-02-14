#include "CountDownWidget.h"

static void setLabel(QPointer<QLabel> label, int seconds)
{
    int hours = seconds / (60 * 60); // 60*60
    int mins = (seconds / 60) % 60;
    int secs = seconds % 60;
    label->setText(QString("%1%2:%3").arg((hours > 0 ? QString("%1:").arg(hours) : "")).arg(mins, 2, 10, QChar('0')).arg(secs, 2, 10, QChar('0')));
}



static void setLabelBackground(QPointer<QLabel> label, QColor color)
{
    static QPalette pal = label->palette();
    pal.setColor(QPalette::Window, color);
    label->setAutoFillBackground(true); // IMPORTANT!
    label->setPalette(pal);
}

static void setRedBackground(QPointer<QLabel> label)
{
    setLabelBackground(label, QColor(Qt::red));
}

static void setBlankBackground(QPointer<QLabel> label)
{
    setLabelBackground(label, QColor(Qt::transparent));
}

CountdownWidget::CountdownWidget(int initialValue) : QWidget()
{
    countdown_seconds_ = initialValue;
    remaining_seconds_ = initialValue;

    timer_ = new QTimer(this);

    /* Inputs */
    input_field_ = new QLineEdit("");
    input_field_->setMaximumWidth(100);
    input_field_->setPlaceholderText("");

    /* Buttons */
    start_BTN = new QPushButton("Hasi");
    setInitialValue_BTN = new QPushButton("Set Initial Timer value");

    /* Labels */
    timer_Label = new QLabel();
    timer_Label->setFont(QFont("Arial", 200));
    timer_Label->setAlignment(Qt::AlignCenter);
    timer_Label->setTextFormat(Qt::RichText); 
    setLabel(timer_Label, initialValue);


    /* Layouts */
    main_layout = new QVBoxLayout(this);
    layout_h = new QHBoxLayout(this);
    main_layout->addWidget(timer_Label);
    main_layout->addLayout(layout_h);
    layout_h->addWidget(input_field_);
    layout_h->addWidget(setInitialValue_BTN);
    main_layout->addWidget(start_BTN);

    /* Connections */
    connect(timer_, &QTimer::timeout, this, &CountdownWidget::updateCountdown);
    connect(start_BTN, &QPushButton::clicked, this, &CountdownWidget::startCountdown);
    connect(setInitialValue_BTN, &QPushButton::clicked, this, std::bind(&CountdownWidget::setInitialValue, this, input_field_));
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
        timer_->stop();
        return;
    }

    countdown_seconds_ = secs;
    setLabel(timer_Label, secs);
    timer_->stop();
}

void CountdownWidget::updateCountdown()
{
    remaining_seconds_ -= 1;
    if (remaining_seconds_ < 0)
    {
        setRedBackground(timer_Label);
        timer_->stop();

        remaining_seconds_ = countdown_seconds_;
        timer_Label->setText("Denbora bukatu da.");
    }
    else
    {
        setBlankBackground(timer_Label);
        int hours = remaining_seconds_ / (60 * 60); // 60*60
        int mins = (remaining_seconds_ / 60) % 60;
        int secs = remaining_seconds_ % 60;
        setLabel(timer_Label, remaining_seconds_);
    }
}

void CountdownWidget::startCountdown()
{
    remaining_seconds_ = countdown_seconds_;
    updateCountdown();
    timer_->start(1000); // Initial delay of 1 second
}
