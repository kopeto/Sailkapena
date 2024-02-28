#ifndef COUNTDOWNWIDGET_H
#define COUNTDOWNWIDGET_H

#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QStyle>
#include <QPointer>
#include <QDebug>

class CountdownWidget : public QWidget
{
    Q_OBJECT
public:
    CountdownWidget(QWidget *parent = nullptr);
    ~CountdownWidget() = default;

    void setInitialValue(QLineEdit *input);
    int getElapsedTime();

    QLabel* public_label{nullptr};

private slots:
    void updateCountdown();
    void startCountdown();
    void toggleHideShow();

private:
    int countdown_seconds_{0};
    int remaining_seconds_{0};

    /* Interface Widgets */
    QPointer<QPushButton> start_BTN;    //  QPushButton *start_BTN;
    QPointer<QPushButton> setInitialValue_BTN; //  QPushButton *setInitialValue_BTN;
    QPointer<QPushButton> hideShowToggle_BTN; 
    QPointer<QLineEdit> input_field_;          //  QLineEdit *input_field_;
    QPointer<QTimer> timer_;                   //  QTimer *timer_;
    QPointer<QLabel> timer_Label;              //  QLabel *label_;

    QPointer<QVBoxLayout> main_layout;
    QPointer<QHBoxLayout> layout_h;
};

#endif // COUNTDOWNWIDGET_H