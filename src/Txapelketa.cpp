#include "Txapelketa.h"

#include "CountDownWidget.h"
#include "PublicWindow.h"
#include "Taula.h"
#include "TimeFormat.h"

#include <QButtonGroup>
#include <QAbstractButton>
#include <QCollator>
#include <QDir>
#include <QDebug>
#include <QFile>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLocale>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>
#include <array>
#include <limits>
#include <utility>

namespace
{
int comparePlayerNames(const QString &left, const QString &right)
{
    static const QCollator basqueCollator(QLocale(QLocale::Basque, QLocale::Spain));
    return basqueCollator.compare(left, right);
}

void showBasqueMessage(QWidget *parent, QMessageBox::Icon icon, const QString &title, const QString &text)
{
    QMessageBox message(icon, title, text, QMessageBox::NoButton, parent);
    message.addButton(QObject::tr("Ados"), QMessageBox::AcceptRole);
    message.exec();
}
}

Txapelketa::Txapelketa(QWidget *parent)
    : QWidget(parent)
{
    table_ = new Taula(this);
    timer_ = new CountdownWidget(this);

    gameSelectorGroup_ = new QButtonGroup(this);
    gameSelectorLayout_ = new QHBoxLayout();
    gameSelectorLayout_->setAlignment(Qt::AlignLeft);
    game1Button_ = new QRadioButton(tr("Lehen proba"), this);
    game2Button_ = new QRadioButton(tr("Bigarren proba"), this);
    finalButton_ = new QRadioButton(tr("Finala"), this);
    updateTableButton_ = new QPushButton(tr("Sailkapena eguneratu"), this);

    game1Button_->setChecked(true);
    gameSelectorGroup_->addButton(game1Button_);
    gameSelectorGroup_->addButton(game2Button_);
    gameSelectorGroup_->addButton(finalButton_);
    gameSelectorLayout_->addWidget(game1Button_);
    gameSelectorLayout_->addWidget(game2Button_);
    gameSelectorLayout_->addWidget(finalButton_);
    gameSelectorLayout_->addWidget(updateTableButton_);

    addPlayerButton_ = new QPushButton(tr("Jokalaria gehitu"), this);
    playerNameInput_ = new QLineEdit(this);
    playerNameInput_->setPlaceholderText(tr("Jokalariaren izena"));
    playerInputLayout_ = new QHBoxLayout();
    playerInputLayout_->addWidget(addPlayerButton_);
    playerInputLayout_->addWidget(playerNameInput_);

    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->addWidget(timer_);
    mainLayout_->addLayout(gameSelectorLayout_);
    mainLayout_->addLayout(playerInputLayout_);
    mainLayout_->addWidget(table_);

    connect(updateTableButton_, &QPushButton::clicked, this, &Txapelketa::updateTable);
    connect(addPlayerButton_, &QPushButton::clicked, this, &Txapelketa::addPlayer);
    connect(playerNameInput_, &QLineEdit::returnPressed, this, &Txapelketa::addPlayer);
    connect(game1Button_, &QRadioButton::clicked, this, [this]() {
        currentGameState_ = GameState::GAME1;
        refreshTable();
    });
    connect(game2Button_, &QRadioButton::clicked, this, [this]() {
        currentGameState_ = GameState::GAME2;
        refreshTable();
    });
    connect(finalButton_, &QRadioButton::clicked, this, [this]() {
        currentGameState_ = GameState::GAME_FINAL;
        refreshTable();
    });
    connect(table_, &Taula::recordTimeRequested, this, &Txapelketa::recordTime);
    connect(table_, &Taula::removePlayerRequested, this, &Txapelketa::removePlayer);
    connect(table_, &Taula::resultEdited, this, &Txapelketa::updateResult);

    const QString applicationDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (!applicationDataPath.isEmpty())
    {
        QDir().mkpath(applicationDataPath);
        checkpointPath_ = applicationDataPath + "/azken_emaitzak.csv";
    }
    if (checkpointPath_.isEmpty())
        checkpointPath_ = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/sailkapenahg_azken_emaitzak.csv";

    refreshTable();
    if (QFile::exists(checkpointPath_))
    {
        QMessageBox message(QMessageBox::Question, tr("Aurreko saioa berreskuratu"),
                            tr("Aurreko lehiaketaren kopia aurkitu da. Berreskuratu nahi duzu?"),
                            QMessageBox::NoButton, this);
        QAbstractButton *restoreButton = message.addButton(tr("Bai"), QMessageBox::AcceptRole);
        QAbstractButton *discardButton = message.addButton(tr("Ez"), QMessageBox::RejectRole);
        message.setDefaultButton(qobject_cast<QPushButton *>(discardButton));
        message.exec();
        if (message.clickedButton() == restoreButton)
            loadFromFile_(checkpointPath_);
    }
}

void Txapelketa::addPlayer()
{
    const QString name = playerNameInput_->text().trimmed();
    if (name.isEmpty())
    {
        showBasqueMessage(this, QMessageBox::Information, tr("Jokalaria gehitu"), tr("Idatzi jokalariaren izena."));
        return;
    }
    if (name.contains(QLatin1Char('\n')) || name.contains(QLatin1Char('\r')))
    {
        showBasqueMessage(this, QMessageBox::Information, tr("Jokalaria gehitu"),
                          tr("Izenak ezin du lerro-jauzirik izan."));
        return;
    }

    const auto duplicate = std::find_if(players_.cbegin(), players_.cend(), [&name](const Player &player) {
        return player.name.compare(name, Qt::CaseInsensitive) == 0;
    });
    if (duplicate != players_.cend())
    {
        showBasqueMessage(this, QMessageBox::Information, tr("Jokalaria gehitu"), tr("Izen hori duen jokalaria badago jada."));
        return;
    }

    players_.emplace_back(name);
    playerNameInput_->clear();
    refreshTable();
    saveCheckpoint();
}

void Txapelketa::updateTable()
{
    refreshTable();
    saveCheckpoint();
}

void Txapelketa::recordTime(int playerId)
{
    Player *player = findPlayer(playerId);
    if (!player)
        return;

    if (!timer_->hasStarted())
    {
        showBasqueMessage(this, QMessageBox::Information, tr("Denbora sartu"), tr("Hasi kronometroa denbora erregistratu aurretik."));
        return;
    }

    const int game = currentGameState_ == GameState::GAME1 ? 0 :
                     currentGameState_ == GameState::GAME2 ? 1 : 2;
    if (game == 2)
    {
        const std::vector<int> finalists = finalistIds();
        if (std::find(finalists.begin(), finalists.end(), playerId) == finalists.end())
            return;
    }

    Result &result = player->results[static_cast<std::size_t>(game)];
    result.timeSeconds = timer_->getElapsedTime();
    if (!result.errors)
        result.errors = 0;

    QTimer::singleShot(0, this, [this]() {
        refreshTable(false);
        saveCheckpoint();
    });
}

void Txapelketa::removePlayer(int playerId)
{
    const auto found = std::find_if(players_.begin(), players_.end(), [playerId](const Player &player) {
        return player.id == playerId;
    });
    if (found == players_.end())
        return;

    players_.erase(found);
    QTimer::singleShot(0, this, [this]() {
        refreshTable();
        saveCheckpoint();
    });
}

void Txapelketa::updateResult(int playerId, int game, int field, const QString &value)
{
    Player *player = findPlayer(playerId);
    if (!player || game < 0 || game >= static_cast<int>(player->results.size()))
        return;

    Result &result = player->results[static_cast<std::size_t>(game)];
    bool valid = false;
    if (field == 0)
    {
        if (value.trimmed().isEmpty())
        {
            result.timeSeconds.reset();
            valid = true;
        }
        else
        {
            const std::optional<int> parsedTime = TimeFormat::parse(value.trimmed());
            valid = parsedTime.has_value();
            if (valid)
                result.timeSeconds = parsedTime;
        }
        if (valid && result.timeSeconds && !result.errors)
            result.errors = 0;
    }
    else if (field == 1)
    {
        if (value.trimmed().isEmpty())
        {
            result.errors.reset();
            valid = true;
        }
        else
        {
            bool ok = false;
            const int errors = value.trimmed().toInt(&ok);
            valid = ok && errors >= 0;
            if (valid)
                result.errors = errors;
        }
    }

    if (!valid)
    {
        const QString message = field == 0
            ? tr("Sartu denbora HH:MM:SS formatuan; adibidez, 00:12:34.")
            : tr("Akats kopuruak zero edo handiagoa den zenbaki osoa izan behar du.");
        QTimer::singleShot(0, this, [this, message]() {
            refreshTable(false);
            showBasqueMessage(this, QMessageBox::Warning, tr("Errorea"), message);
        });
        return;
    }

    QTimer::singleShot(0, this, [this]() {
        refreshTable(false);
        saveCheckpoint();
    });
}

void Txapelketa::togglePublicWindow()
{
    if (!publicWindow_)
    {
        publicWindow_ = new PublicWindow(this);
        timer_->setPublicLabel(publicWindow_->timerLabel());
    }

    if (publicWindow_->isVisible())
        publicWindow_->hide();
    else
        publicWindow_->show();
}

void Txapelketa::refreshTable(bool sortByRanking)
{
    const std::vector<int> finalists = finalistIds();
    std::vector<Player> ranking;
    if (sortByRanking)
    {
        ranking = preliminaryRanking();
    }
    else
    {
        ranking = currentDisplayOrder();
    }

    if (sortByRanking && currentGameState_ == GameState::GAME_FINAL)
    {
        std::vector<Player> finalRanked;
        std::vector<Player> remaining;
        for (const Player &player : ranking)
        {
            const bool isFinalist = std::find(finalists.begin(), finalists.end(), player.id) != finalists.end();
            (isFinalist ? finalRanked : remaining).push_back(player);
        }

        std::stable_sort(finalRanked.begin(), finalRanked.end(), [](const Player &left, const Player &right) {
            const Result &leftResult = left.results[2];
            const Result &rightResult = right.results[2];
            const bool leftHasTime = leftResult.timeSeconds.has_value();
            const bool rightHasTime = rightResult.timeSeconds.has_value();
            if (leftHasTime != rightHasTime)
                return leftHasTime;

            const int leftErrors = leftHasTime ? leftResult.errors.value_or(0) : std::numeric_limits<int>::max();
            const int rightErrors = rightHasTime ? rightResult.errors.value_or(0) : std::numeric_limits<int>::max();
            if (leftErrors != rightErrors)
                return leftErrors < rightErrors;

            const int leftTime = leftResult.timeSeconds.value_or(std::numeric_limits<int>::max());
            const int rightTime = rightResult.timeSeconds.value_or(std::numeric_limits<int>::max());
            if (leftTime != rightTime)
                return leftTime < rightTime;
            return comparePlayerNames(left.name, right.name) < 0;
        });

        finalRanked.insert(finalRanked.end(), remaining.begin(), remaining.end());
        ranking = std::move(finalRanked);
    }

    table_->setPlayers(ranking, finalists, currentGameState_ == GameState::GAME_FINAL);
}

std::vector<Player> Txapelketa::currentDisplayOrder() const
{
    std::vector<Player> orderedPlayers;
    std::vector<int> visibleIds;
    visibleIds.reserve(static_cast<std::size_t>(table_->rowCount()));
    orderedPlayers.reserve(players_.size());

    for (int row = 0; row < table_->rowCount(); ++row)
    {
        const QTableWidgetItem *nameItem = table_->item(row, Taula::PLAYER_NAME_COLUMN);
        if (nameItem)
            visibleIds.push_back(nameItem->data(Qt::UserRole).toInt());
    }
    for (int id : visibleIds)
    {
        const Player *player = findPlayer(id);
        if (player)
            orderedPlayers.push_back(*player);
    }
    for (const Player &player : players_)
    {
        if (std::find(visibleIds.begin(), visibleIds.end(), player.id) == visibleIds.end())
            orderedPlayers.push_back(player);
    }
    return orderedPlayers;
}

std::vector<Player> Txapelketa::preliminaryRanking() const
{
    std::vector<Player> ranking = players_;
    auto score = [](const Player &player) {
        qint64 completedRounds = 0;
        qint64 totalErrors = 0;
        qint64 totalTime = 0;
        for (int game = 0; game < 2; ++game)
        {
            const Result &result = player.results[static_cast<std::size_t>(game)];
            if (result.timeSeconds)
            {
                ++completedRounds;
                totalTime += *result.timeSeconds;
                totalErrors += result.errors.value_or(0);
            }
        }
        return std::array<qint64, 3>{{completedRounds, totalErrors, totalTime}};
    };

    std::stable_sort(ranking.begin(), ranking.end(), [&score](const Player &left, const Player &right) {
        const auto leftScore = score(left);
        const auto rightScore = score(right);
        if (leftScore[0] != rightScore[0])
            return leftScore[0] > rightScore[0];
        if (leftScore[1] != rightScore[1])
            return leftScore[1] < rightScore[1];
        if (leftScore[2] != rightScore[2])
            return leftScore[2] < rightScore[2];
        return comparePlayerNames(left.name, right.name) < 0;
    });
    return ranking;
}

std::vector<int> Txapelketa::finalistIds() const
{
    const std::vector<Player> ranking = preliminaryRanking();
    std::vector<int> ids;
    const std::size_t count = std::min<std::size_t>(FINALISTS_COUNT, ranking.size());
    ids.reserve(count);
    for (std::size_t index = 0; index < count; ++index)
        ids.push_back(ranking[index].id);
    return ids;
}

Player *Txapelketa::findPlayer(int playerId)
{
    const auto found = std::find_if(players_.begin(), players_.end(), [playerId](const Player &player) {
        return player.id == playerId;
    });
    return found == players_.end() ? nullptr : &*found;
}

const Player *Txapelketa::findPlayer(int playerId) const
{
    const auto found = std::find_if(players_.cbegin(), players_.cend(), [playerId](const Player &player) {
        return player.id == playerId;
    });
    return found == players_.cend() ? nullptr : &*found;
}

void Txapelketa::saveCheckpoint()
{
    if (checkpointPath_.isEmpty())
        return;
    if (saveToFile_(checkpointPath_))
    {
        checkpointFailureReported_ = false;
        return;
    }

    qWarning() << "Ezin izan da azken emaitzen kopia gorde:" << checkpointPath_;
    if (!checkpointFailureReported_)
    {
        checkpointFailureReported_ = true;
        showBasqueMessage(this, QMessageBox::Warning, tr("Kopia automatikoa"),
                          tr("Ezin izan da azken emaitzen kopia gorde. Gorde emaitzak fitxategi batean."));
    }
}
