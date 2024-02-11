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
public:
    CountdownWidget(int countdown_seconds);

    void setInitialValue(QLineEdit* input);

private slots:
    void updateCountdown();
    void startCountdown();

private:
    int countdown_seconds_;
    int remaining_seconds_;


   QPointer<QPushButton>    start_BTN;             //  QPushButton *start_BTN;
   QPointer<QPushButton>    setInitialValue_BTN;   //  QPushButton *setInitialValue_BTN;
   QPointer<QLineEdit>      input_field_;          //  QLineEdit *input_field_;
   QPointer<QTimer>         timer_;                //  QTimer *timer_;
   QPointer<QLabel>         label_;                //  QLabel *label_;

   QPointer<QVBoxLayout> main_layout;
   QPointer<QHBoxLayout> layout_h;

   QPointer<QDebug> debug;                         //  QDebug *debug;
};
