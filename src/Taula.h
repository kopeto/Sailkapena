#ifndef TAULA_H
#define TAULA_H

#include <QList>
#include <QTableWidget>

#include <array>
#include <vector>

#include "Player.h"
#include "TournamentFormat.h"

class QKeyEvent;
class QPaintEvent;

class Taula : public QTableWidget
{
    Q_OBJECT

public:
    static constexpr int PLAYER_ACTION_COLUMN = 0;
    static constexpr int PLAYER_NAME_COLUMN = 1;

    explicit Taula(QWidget *parent = nullptr);
    void setPublicDisplay(bool publicDisplay);
    void setPlayers(const std::vector<Player> &players,
                    const QVector<RoundDefinition> &rounds,
                    const std::vector<int> &eligibleIds,
                    const std::vector<std::vector<int>> &eligibleIdsByRound,
                    int activeRound,
                    bool allowEditing);

signals:
    void recordTimeRequested(int playerId);
    void removePlayersRequested(const QList<int> &playerIds);
    void editPlayerNameRequested(int playerId);
    void resultEdited(int playerId, int round, int field, const QString &value);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    QList<int> selectedPlayerIds() const;
    void confirmAndRemove(const QList<int> &playerIds);
    bool publicDisplay_{false};
    std::vector<std::array<int, 2>> roundColumns_;
    std::vector<std::array<int, 2>> cutColumns_;
    std::vector<int> separatorColumns_;
};

#endif // TAULA_H
