#ifndef TAULA_H
#define TAULA_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QKeyEvent>

#include "Player.h"

#define PLAYER_ID_COLUMN    1
#define PLAYER_NAME_COLUMN  2
#define GAME1_TIME_COLUMN   3
#define GAME1_ERRORS_COLUMN 4
#define GAME2_TIME_COLUMN   5
#define GAME2_ERRORS_COLUMN 6
#define GAME3_TIME_COLUMN   7
#define GAME3_ERRORS_COLUMN 8
#define TOTAL_TIME_COLUMN   9
#define TOTAL_ERRORS_COLUMN 10

class Taula: public QTableWidget
{
    Q_OBJECT
public:
    Taula(QWidget *parent = nullptr);
    ~Taula() = default;

    void addPlayer(const Player &player);

    QString getPlayerNameFromRow(int row);
    bool checkPlayerExists(const QString& name);

    void deleteRow(int row);

    int getRowTotalTimeInternal(int row);
    int getRowTotalTimeReal(int row);

    int getRowTotalErrorsInternal(int row);
    int getRowTotalErrorsReal(int row);

    protected:
    void keyPressEvent(QKeyEvent *event) override ;
};


#endif