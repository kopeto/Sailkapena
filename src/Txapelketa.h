#ifndef TXAPELKETA_H
#define TXAPELKETA_H

#include <QObject>
#include <QDebug>
#include <QPointer>
#include <QStandardItemModel>
#include <QRadioButton>
#include <QButtonGroup>

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

    QPointer<Taula> taula;
    QPointer<QVBoxLayout> mainLayout;
    QPointer<CountdownWidget> timer;

    /* Interface components */
    /* game selector */
    QPointer<QButtonGroup>  GameSelectorGroup;
    QPointer<QHBoxLayout>   GameSelectorLayout;
    QPointer<QRadioButton>  game_1;
    QPointer<QRadioButton>  game_2;
    QPointer<QRadioButton>  game_final;

    QPointer<QPushButton> updateTable_BTN;
    QPointer<QPushButton> addPlayer_BTN; //  QPushButton *setInitialValue_BTN;
    QPointer<QLineEdit>     input_player_name;
    QPointer<QHBoxLayout> layout_h;

    std::vector<Player> Players;

    void printPlayers();

    void updateTable();
    std::pair<int, int> getCurrentGameColumns();

public slots:
    void deletePlayer(const QString& name);
    void addPlayer();
    
    // void setTime(const Player  &player);
    void setTime(const QString &name);

private:
    void updateInternals_();
    void updateReals_();
};

#endif // TXAPELKETA_H