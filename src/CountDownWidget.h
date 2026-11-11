#ifndef COUNTDOWNWIDGET_H
#define COUNTDOWNWIDGET_H

#include <QElapsedTimer>
#include <QLabel>
#include <QPointer>
#include <QTimer>
#include <QString>
#include <QWidget>

class QHBoxLayout;
class QLineEdit;
class QPushButton;
class QVBoxLayout;

class CountdownWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CountdownWidget(QWidget *parent = nullptr);

    int getElapsedTime() const;
    bool hasStarted() const;
    void setPublicLabel(QLabel *label);
    void configureRound(int durationMinutes, const QString &roundName);

    QPointer<QLabel> public_label;

private slots:
    void updateCountdown();
    void startCountdown();
    void setInitialValue();
    void toggleHideShow();

private:
    int countdownSeconds_{0};
    int remainingSeconds_{0};
    bool hasStarted_{false};
    QElapsedTimer elapsedClock_;

    QPointer<QPushButton> startButton_;
    QPointer<QPushButton> setInitialValueButton_;
    QPointer<QPushButton> hideShowToggleButton_;
    QPointer<QLineEdit> inputField_;
    QPointer<QTimer> timer_;
    QPointer<QLabel> timerLabel_;
    QPointer<QLabel> roundLabel_;
    QPointer<QVBoxLayout> mainLayout_;
    QPointer<QHBoxLayout> horizontalLayout_;
};

#endif // COUNTDOWNWIDGET_H
