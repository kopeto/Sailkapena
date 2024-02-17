#ifndef TXAPELKETA_H
#define TXAPELKETA_H

#include <QObject>
#include <QDebug>
#include <QPointer>
#include <QStandardItemModel>

#include <vector>

#include "Player.h"
#include "Taula.h"
#include "CountDownWidget.h"

class Txapelketa : public QWidget
{
Q_OBJECT

public:
    Txapelketa(QWidget *parent = nullptr);
    ~Txapelketa() = default;

    QPointer<Taula>                   taula;
    QPointer<QVBoxLayout>             mainLayout;
    QPointer<CountdownWidget>         timer;

    /* Interface components */
    QPointer<QPushButton>   addPlayer_BTN;      //  QPushButton *setInitialValue_BTN;
    QPointer<QLineEdit>     input_player_name;
    QPointer<QHBoxLayout>   layout_h;

    std::vector<Player> Players;
    
    void printPlayers();

    void setTime(const QString& player_name);

public slots:
    void addPlayer();

private:
   void updateTable();
};

#endif // TXAPELKETA_H