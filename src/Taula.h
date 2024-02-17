#ifndef TAULA_H
#define TAULA_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QHBoxLayout>

#include "Player.h"

#define PLAYER_ID_COLUMN    1
#define PLAYER_NAME_COLUMN  2

class Taula: public QTableWidget
{
    Q_OBJECT
public:
    Taula(QWidget *parent = nullptr);
    ~Taula() = default;

    void addPlayer(const Player &player);
    void setTime();
};


#endif