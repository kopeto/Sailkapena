#ifndef PUBLICWINDOW_H
#define PUBLICWINDOW_H

#include <QWidget>
#include <QLabel>

class QVBoxLayout;
class Taula;

class PublicWindow : public QWidget
{
    Q_OBJECT
public:
    explicit PublicWindow(QWidget *parent = nullptr);
    QLabel *timerLabel() const;
    Taula *standingsTable() const;

private:
    QLabel *timerLabel_;
    Taula *standingsTable_;
    QVBoxLayout *layout_;
};

#endif
