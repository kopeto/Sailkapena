#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>

class CountdownWidget : public QWidget
{
public:
    CountdownWidget(int countdown_seconds);

    void setInitialValue(int secs);

private slots:
    void updateCountdown();
    void startCountdown();

private:
    int countdown_seconds_;
    int remaining_seconds_;


    QPushButton *start_button_;
    QLineEdit *input_field_;
    QTimer *timer_;
    QLabel *label_;
};
