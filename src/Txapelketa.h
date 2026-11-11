#ifndef TXAPELKETA_H
#define TXAPELKETA_H

#include <QPointer>
#include <QComboBox>
#include <QStandardPaths>
#include <QString>
#include <QWidget>

#include <vector>

#include "Player.h"
#include "TournamentFormat.h"

class QHBoxLayout;
class QLabel;
class QLineEdit;
class QPushButton;
class QVBoxLayout;
class Taula;
class CountdownWidget;
class PublicWindow;

class Txapelketa : public QWidget
{
    Q_OBJECT

public:
    explicit Txapelketa(QWidget *parent = nullptr);

    bool saveToFile();
    bool loadFromFile();
    bool exportToCsv();

public slots:
    void addPlayer();
    void togglePublicWindow();
    void editTournamentFormat();

private slots:
    void updateTable();
    void recordTime(int playerId);
    void removePlayer(int playerId);
    void updateResult(int playerId, int round, int field, const QString &value);
    void finishCurrentRound();
    void selectRound(int roundIndex);

private:
    void refreshTable(bool sortByRanking = true);
    std::vector<Player> rankedEligiblePlayers(int round) const;
    std::vector<int> eligiblePlayerIds(int round) const;
    bool isResultComplete(const Player &player, int round) const;
    int compareScores(const Player &left, const Player &right, int round) const;
    bool hasTournamentResults() const;
    void applyDefinition(const TournamentDefinition &definition);
    void updateRoundControls();
    Player *findPlayer(int playerId);
    const Player *findPlayer(int playerId) const;
    void saveCheckpoint();
    bool loadFromFile_(const QString &filePath);
    bool saveToFile_(const QString &filePath) const;

    QPointer<Taula> table_;
    QPointer<CountdownWidget> timer_;
    QPointer<QVBoxLayout> mainLayout_;
    QPointer<QHBoxLayout> roundControlLayout_;
    QPointer<QPushButton> finishRoundButton_;
    QPointer<QPushButton> updateTableButton_;
    QPointer<QPushButton> addPlayerButton_;
    QPointer<QPushButton> editFormatButton_;
    QPointer<QLabel> currentRoundLabel_;
    QPointer<QComboBox> roundSelector_;
    QPointer<QLineEdit> playerNameInput_;
    QPointer<QHBoxLayout> playerInputLayout_;
    QPointer<PublicWindow> publicWindow_;

    TournamentDefinition definition_;
    std::vector<Player> players_;
    std::vector<std::vector<int>> qualifiedIdsByRound_;
    int activeRoundIndex_{0};
    bool tournamentFinished_{false};
    QString checkpointPath_;
    QString legacyCheckpointPath_;
    bool checkpointFailureReported_{false};
};

#endif // TXAPELKETA_H
