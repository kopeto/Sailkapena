#ifndef TXAPELKETA_H
#define TXAPELKETA_H

#include <QObject>
#include <QDebug>
#include <QPointer>
#include <QStandardItemModel>
#include <QRadioButton>
#include <QButtonGroup>
#include <QString>
#include <QVBoxLayout>
#include <QWindow>
#include <QLineEdit>

#include <vector>

#include "Player.h"
#include "PublicWindow.h"

class Taula;
class CountdownWidget;

enum class GameState
{
    GAME1,
    GAME2,
    GAME_FINAL
};

class Txapelketa : public QWidget
{
    Q_OBJECT

static constexpr int FINALISTS_COUNT = 6;

public:

    Txapelketa(QWidget *parent = nullptr);
    ~Txapelketa() = default;

    QPointer<Taula> taula;
    QPointer<QVBoxLayout> mainLayout;
    QPointer<CountdownWidget> timer;

    /* Interface components */
    /* game selector */
    QPointer<QButtonGroup> GameSelectorGroup;
    QPointer<QHBoxLayout> GameSelectorLayout;
    QPointer<QRadioButton> game_1;
    QPointer<QRadioButton> game_2;
    QPointer<QRadioButton> game_final;

    QPointer<QPushButton> updateTable_BTN;
    QPointer<QPushButton> addPlayer_BTN; //  QPushButton *setInitialValue_BTN;
    QPointer<QLineEdit> input_player_name;
    QPointer<QHBoxLayout> layout_h;
    QPointer<QHBoxLayout> layout_table;

    // // solution image
    // QPointer<QLabel> image_label;
    // QPixmap *image_pixmap;

    /**
     * Public window
     */
    // QPointer<PublicWindow> publicwindow;

    std::vector<Player> Players;

    void printPlayers();

    void updateTable();
    std::pair<int, int> getCurrentGameColumns();

    /* IO read write functions */
    bool saveToFile();
    bool loadFromFile();
    bool saveToExcel();

public slots:
    void deletePlayer(const QString &name);
    void addPlayerNamed(const QString& name);
    void addPlayer();

    // void setTime(const Player  &player);
    void setTime(const QString &name);

private:
    void updateInternals_();
    void updateReals_();
    void checkDraws_();
    bool saveToFile_( const QString& filePath );
    QString _tmpResultsFilePath{"/tmp/txapelketa_results.tmp"};
    GameState currentGameState{GameState::GAME1};
};

#endif // TXAPELKETA_H