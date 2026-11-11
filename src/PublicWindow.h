#ifndef PUBLICWINDOW_H
#define PUBLICWINDOW_H

#include <QWidget>
#include <QLabel>

class QVBoxLayout;

class PublicWindow : public QWidget
{
    Q_OBJECT
public:
    explicit PublicWindow(QWidget *parent = nullptr);
    QLabel *timerLabel() const;

private:
    QLabel *timerLabel_;
    QVBoxLayout *layout_;
};

#endif
