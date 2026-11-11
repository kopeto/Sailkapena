#ifndef TAULA_H
#define TAULA_H

#include <QTableWidget>

#include <array>
#include <vector>

#include "Player.h"
#include "TournamentFormat.h"

class QKeyEvent;

class Taula : public QTableWidget
{
    Q_OBJECT

public:
    static constexpr int PLAYER_ACTION_COLUMN = 0;
    static constexpr int PLAYER_NAME_COLUMN = 1;

    explicit Taula(QWidget *parent = nullptr);
    void setPlayers(const std::vector<Player> &players,
                    const QVector<RoundDefinition> &rounds,
                    const std::vector<int> &eligibleIds,
                    int activeRound,
                    bool allowEditing);

signals:
    void recordTimeRequested(int playerId);
    void removePlayerRequested(int playerId);
    void resultEdited(int playerId, int round, int field, const QString &value);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    std::vector<std::array<int, 2>> roundColumns_;
    std::vector<std::array<int, 2>> cutColumns_;
};

#endif // TAULA_H
