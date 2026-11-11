#ifndef TXAPELKETA_H
#define TXAPELKETA_H

#include <QPointer>
#include <QStandardPaths>
#include <QString>
#include <QWidget>

#include <vector>

#include "Player.h"

class QButtonGroup;
class QHBoxLayout;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QVBoxLayout;
class Taula;
class CountdownWidget;
class PublicWindow;

enum class GameState
{
    GAME1,
    GAME2,
    GAME_FINAL
};

class Txapelketa : public QWidget
{
    Q_OBJECT

public:
    static constexpr int FINALISTS_COUNT = 6;

    explicit Txapelketa(QWidget *parent = nullptr);

    bool saveToFile();
    bool loadFromFile();
    bool exportToCsv();

public slots:
    void addPlayer();
    void togglePublicWindow();

private slots:
    void updateTable();
    void recordTime(int playerId);
    void removePlayer(int playerId);
    void updateResult(int playerId, int game, int field, const QString &value);

private:
    void refreshTable(bool sortByRanking = true);
    std::vector<Player> preliminaryRanking() const;
    std::vector<Player> currentDisplayOrder() const;
    std::vector<int> finalistIds() const;
    Player *findPlayer(int playerId);
    const Player *findPlayer(int playerId) const;
    void saveCheckpoint();
    bool loadFromFile_(const QString &filePath);
    bool saveToFile_(const QString &filePath) const;

    QPointer<Taula> table_;
    QPointer<CountdownWidget> timer_;
    QPointer<QVBoxLayout> mainLayout_;
    QPointer<QButtonGroup> gameSelectorGroup_;
    QPointer<QHBoxLayout> gameSelectorLayout_;
    QPointer<QRadioButton> game1Button_;
    QPointer<QRadioButton> game2Button_;
    QPointer<QRadioButton> finalButton_;
    QPointer<QPushButton> updateTableButton_;
    QPointer<QPushButton> addPlayerButton_;
    QPointer<QLineEdit> playerNameInput_;
    QPointer<QHBoxLayout> playerInputLayout_;
    QPointer<PublicWindow> publicWindow_;

    std::vector<Player> players_;
    GameState currentGameState_{GameState::GAME1};
    QString checkpointPath_;
    bool checkpointFailureReported_{false};
};

#endif // TXAPELKETA_H
