
#include <QLabel>
#include <QPointer>
#include <QColor>
#include <QPalette>

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
