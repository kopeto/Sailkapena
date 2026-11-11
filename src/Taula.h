#ifndef TAULA_H
#define TAULA_H

#include <QTableWidget>

#include <vector>

#include "Player.h"

class QKeyEvent;

class Taula : public QTableWidget
{
    Q_OBJECT

public:
    static constexpr int PLAYER_ACTION_COLUMN = 0;
    static constexpr int PLAYER_NAME_COLUMN = 1;
    static constexpr int GAME1_TIME_COLUMN = 2;
    static constexpr int GAME1_ERRORS_COLUMN = 3;
    static constexpr int GAME2_TIME_COLUMN = 4;
    static constexpr int GAME2_ERRORS_COLUMN = 5;
    static constexpr int TOTAL_TIME_COLUMN = 6;
    static constexpr int TOTAL_ERRORS_COLUMN = 7;
    static constexpr int FINALS_SEPARATOR_COLUMN = 8;
    static constexpr int FINALS_TIME_COLUMN = 9;
    static constexpr int FINALS_ERRORS_COLUMN = 10;

    explicit Taula(QWidget *parent = nullptr);
    void setPlayers(const std::vector<Player> &players, const std::vector<int> &finalistIds, bool finalMode);

signals:
    void recordTimeRequested(int playerId);
    void removePlayerRequested(int playerId);
    void resultEdited(int playerId, int game, int field, const QString &value);

protected:
    void keyPressEvent(QKeyEvent *event) override;
};

#endif // TAULA_H
