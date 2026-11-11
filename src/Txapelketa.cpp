#include "Txapelketa.h"

#include "CountDownWidget.h"
#include "PublicWindow.h"
#include "Taula.h"
#include "TimeFormat.h"

#include <QAbstractButton>
#include <QCollator>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QLocale>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSaveFile>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>
#include <limits>

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

bool roundHasCriterion(const RoundDefinition &round, ResultCriterion criterion)
{
    return std::find(round.criteria.cbegin(), round.criteria.cend(), criterion) != round.criteria.cend();
}
}

Txapelketa::Txapelketa(QWidget *parent)
    : QWidget(parent), definition_(TournamentDefinition::defaultDefinition())
{
    table_ = new Taula(this);
    timer_ = new CountdownWidget(this);
    currentRoundLabel_ = new QLabel(this);
    currentRoundLabel_->setStyleSheet(QStringLiteral("font-size: 18px; font-weight: bold;"));

    finishRoundButton_ = new QPushButton(this);
    updateTableButton_ = new QPushButton(tr("Sailkapena eguneratu"), this);
    editFormatButton_ = new QPushButton(tr("Txapelketaren formatua editatu (JSON)"), this);
    roundControlLayout_ = new QHBoxLayout();
    roundControlLayout_->setAlignment(Qt::AlignLeft);
    roundControlLayout_->addWidget(currentRoundLabel_);
    roundControlLayout_->addWidget(finishRoundButton_);
    roundControlLayout_->addWidget(updateTableButton_);
    roundControlLayout_->addWidget(editFormatButton_);

    addPlayerButton_ = new QPushButton(tr("Jokalaria gehitu"), this);
    playerNameInput_ = new QLineEdit(this);
    playerNameInput_->setPlaceholderText(tr("Jokalariaren izena"));
    playerInputLayout_ = new QHBoxLayout();
    playerInputLayout_->addWidget(addPlayerButton_);
    playerInputLayout_->addWidget(playerNameInput_);

    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->addWidget(timer_);
    mainLayout_->addLayout(roundControlLayout_);
    mainLayout_->addLayout(playerInputLayout_);
    mainLayout_->addWidget(table_);

    connect(finishRoundButton_, &QPushButton::clicked, this, &Txapelketa::finishCurrentRound);
    connect(updateTableButton_, &QPushButton::clicked, this, &Txapelketa::updateTable);
    connect(editFormatButton_, &QPushButton::clicked, this, &Txapelketa::editTournamentFormat);
    connect(addPlayerButton_, &QPushButton::clicked, this, &Txapelketa::addPlayer);
    connect(playerNameInput_, &QLineEdit::returnPressed, this, &Txapelketa::addPlayer);
    connect(table_, &Taula::recordTimeRequested, this, &Txapelketa::recordTime);
    connect(table_, &Taula::removePlayerRequested, this, &Txapelketa::removePlayer);
    connect(table_, &Taula::resultEdited, this, &Txapelketa::updateResult);

    timer_->configureRound(definition_.rounds.first().durationMinutes, definition_.rounds.first().name);

    qualifiedIdsByRound_.resize(static_cast<std::size_t>(definition_.rounds.size()));
    const QString applicationDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (!applicationDataPath.isEmpty())
    {
        QDir().mkpath(applicationDataPath);
        checkpointPath_ = applicationDataPath + "/azken_emaitzak.json";
        legacyCheckpointPath_ = applicationDataPath + "/azken_emaitzak.csv";
    }
    if (checkpointPath_.isEmpty())
    {
        const QString temporaryPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
        checkpointPath_ = temporaryPath + "/sailkapenahg_azken_emaitzak.json";
        legacyCheckpointPath_ = temporaryPath + "/sailkapenahg_azken_emaitzak.csv";
    }

    updateRoundControls();
    refreshTable();
    const QString checkpointToRestore = QFile::exists(checkpointPath_) ? checkpointPath_ : legacyCheckpointPath_;
    if (QFile::exists(checkpointToRestore))
    {
        QMessageBox message(QMessageBox::Question, tr("Aurreko saioa berreskuratu"),
                            tr("Aurreko lehiaketaren kopia aurkitu da. Berreskuratu nahi duzu?"),
                            QMessageBox::NoButton, this);
        QAbstractButton *restoreButton = message.addButton(tr("Bai"), QMessageBox::AcceptRole);
        QAbstractButton *discardButton = message.addButton(tr("Ez"), QMessageBox::RejectRole);
        message.setDefaultButton(qobject_cast<QPushButton *>(discardButton));
        message.exec();
        if (message.clickedButton() == restoreButton)
            loadFromFile_(checkpointToRestore);
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

    Player player(name);
    player.results.resize(static_cast<std::size_t>(definition_.rounds.size()));
    players_.push_back(std::move(player));
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
    if (!player || tournamentFinished_ || activeRoundIndex_ < 0 || activeRoundIndex_ >= definition_.rounds.size())
        return;
    const std::vector<int> eligibleIds = eligiblePlayerIds(activeRoundIndex_);
    if (std::find(eligibleIds.cbegin(), eligibleIds.cend(), playerId) == eligibleIds.cend())
        return;
    if (!timer_->hasStarted())
    {
        showBasqueMessage(this, QMessageBox::Information, tr("Denbora sartu"),
                          tr("Hasi kronometroa denbora erregistratu aurretik."));
        return;
    }

    Result &result = player->results[static_cast<std::size_t>(activeRoundIndex_)];
    result.timeSeconds = timer_->getElapsedTime();
    if (roundHasCriterion(definition_.rounds.at(activeRoundIndex_), ResultCriterion::Errors) && !result.errors)
        result.errors = 0;
    refreshTable(false);
    saveCheckpoint();
}

void Txapelketa::removePlayer(int playerId)
{
    if (tournamentFinished_)
    {
        showBasqueMessage(this, QMessageBox::Information, tr("Jokalaria ezabatu"),
                          tr("Txapelketa amaitu ondoren ezin da jokalaririk ezabatu."));
        return;
    }
    const auto found = std::find_if(players_.begin(), players_.end(), [playerId](const Player &player) {
        return player.id == playerId;
    });
    if (found == players_.end())
        return;
    if (activeRoundIndex_ > 0)
    {
        const std::vector<int> eligibleIds = eligiblePlayerIds(activeRoundIndex_);
        const bool isActivePlayer = std::find(eligibleIds.cbegin(), eligibleIds.cend(), playerId) != eligibleIds.cend();
        if (isActivePlayer && eligibleIds.size() == 1)
        {
            showBasqueMessage(this, QMessageBox::Information, tr("Jokalaria ezabatu"),
                              tr("Ezin da txandako azken jokalaria ezabatu."));
            return;
        }
    }
    players_.erase(found);
    for (std::vector<int> &qualifiedIds : qualifiedIdsByRound_)
        qualifiedIds.erase(std::remove(qualifiedIds.begin(), qualifiedIds.end(), playerId), qualifiedIds.end());
    refreshTable();
    saveCheckpoint();
}

void Txapelketa::updateResult(int playerId, int round, int field, const QString &value)
{
    Player *player = findPlayer(playerId);
    if (!player || tournamentFinished_ || round != activeRoundIndex_ || round < 0 || round >= definition_.rounds.size())
        return;
    const std::vector<int> eligibleIds = eligiblePlayerIds(round);
    if (std::find(eligibleIds.cbegin(), eligibleIds.cend(), playerId) == eligibleIds.cend())
        return;
    const ResultCriterion criterion = field == 0 ? ResultCriterion::Time : ResultCriterion::Errors;
    if ((field != 0 && field != 1) || !roundHasCriterion(definition_.rounds.at(round), criterion))
        return;

    Result &result = player->results[static_cast<std::size_t>(round)];
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
    }
    else if (value.trimmed().isEmpty())
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

void Txapelketa::finishCurrentRound()
{
    if (tournamentFinished_ || activeRoundIndex_ < 0 || activeRoundIndex_ >= definition_.rounds.size())
        return;

    const std::vector<int> eligibleIds = eligiblePlayerIds(activeRoundIndex_);
    if (eligibleIds.empty())
    {
        showBasqueMessage(this, QMessageBox::Information, tr("Txanda amaitu"),
                          tr("Txanda honetan ez dago jokalaririk."));
        return;
    }
    for (int playerId : eligibleIds)
    {
        const Player *player = findPlayer(playerId);
        if (!player || !isResultComplete(*player, activeRoundIndex_))
        {
            showBasqueMessage(this, QMessageBox::Warning, tr("Emaitzak falta dira"),
                              tr("Jokalari guztien emaitzak osatu behar dira txanda itxi aurretik."));
            return;
        }
    }

    if (activeRoundIndex_ == definition_.rounds.size() - 1)
    {
        tournamentFinished_ = true;
        updateRoundControls();
        refreshTable();
        saveCheckpoint();
        showBasqueMessage(this, QMessageBox::Information, tr("Txapelketa amaitu"),
                          tr("Azken sailkapena prest dago."));
        return;
    }

    const RoundDefinition &round = definition_.rounds.at(activeRoundIndex_);
    const std::vector<Player> ranking = rankedEligiblePlayers(activeRoundIndex_);
    int advanceCount = round.advanceCount ? std::min<int>(*round.advanceCount, static_cast<int>(ranking.size()))
                                          : static_cast<int>(ranking.size());
    if (advanceCount == 0)
        return;

    int cutoffEnd = advanceCount;
    while (cutoffEnd < static_cast<int>(ranking.size()) &&
           compareScores(ranking[static_cast<std::size_t>(advanceCount - 1)],
                         ranking[static_cast<std::size_t>(cutoffEnd)], activeRoundIndex_) == 0)
        ++cutoffEnd;

    std::vector<int> advancingIds;
    if (cutoffEnd == advanceCount || round.tiePolicy == TiePolicy::IncludeTies)
    {
        const int includedCount = round.tiePolicy == TiePolicy::IncludeTies ? cutoffEnd : advanceCount;
        for (int index = 0; index < includedCount; ++index)
            advancingIds.push_back(ranking[static_cast<std::size_t>(index)].id);
    }
    else
    {
        const int tieStart = advanceCount - 1;
        QDialog dialog(this);
        QListWidget *tieList = new QListWidget(&dialog);
        for (int index = tieStart; index < cutoffEnd; ++index)
        {
            const Player &player = ranking[static_cast<std::size_t>(index)];
            auto *item = new QListWidgetItem(player.name, tieList);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(Qt::Unchecked);
            item->setData(Qt::UserRole, player.id);
        }

        dialog.setWindowTitle(tr("Berdinketa ebatzi"));
        auto *layout = new QVBoxLayout(&dialog);
        layout->addWidget(new QLabel(tr("Aukeratu %1 jokalari berdinketa hausten duen taldetik.")
                                         .arg(advanceCount - tieStart), &dialog));
        layout->addWidget(tieList);
        auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
        buttons->button(QDialogButtonBox::Ok)->setText(tr("Aurrera egin"));
        buttons->button(QDialogButtonBox::Cancel)->setText(tr("Utzi"));
        layout->addWidget(buttons);
        connect(buttons, &QDialogButtonBox::accepted, &dialog, [&dialog, tieList, needed = advanceCount - tieStart]() {
            int selected = 0;
            for (int row = 0; row < tieList->count(); ++row)
                selected += tieList->item(row)->checkState() == Qt::Checked ? 1 : 0;
            if (selected != needed)
            {
                showBasqueMessage(&dialog, QMessageBox::Information, QObject::tr("Aukeraketa"),
                                  QObject::tr("Hain zuzen %1 jokalari aukeratu behar dituzu.").arg(needed));
                return;
            }
            dialog.accept();
        });
        connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
        if (dialog.exec() != QDialog::Accepted)
            return;

        for (int index = 0; index < tieStart; ++index)
            advancingIds.push_back(ranking[static_cast<std::size_t>(index)].id);
        for (int row = 0; row < tieList->count(); ++row)
        {
            const QListWidgetItem *item = tieList->item(row);
            if (item->checkState() == Qt::Checked)
                advancingIds.push_back(item->data(Qt::UserRole).toInt());
        }
    }

    qualifiedIdsByRound_[static_cast<std::size_t>(activeRoundIndex_ + 1)] = std::move(advancingIds);
    ++activeRoundIndex_;
    timer_->configureRound(definition_.rounds.at(activeRoundIndex_).durationMinutes,
                           definition_.rounds.at(activeRoundIndex_).name);
    updateRoundControls();
    refreshTable();
    saveCheckpoint();
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

void Txapelketa::editTournamentFormat()
{
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Txapelketaren formatua editatu"));
    dialog.resize(720, 620);
    auto *layout = new QVBoxLayout(&dialog);
    auto *help = new QLabel(tr("Editatu JSON formatua. Irizpideen zerrendako ordenak sailkapeneko lehentasuna adierazten du."), &dialog);
    help->setWordWrap(true);
    layout->addWidget(help);
    auto *editor = new QPlainTextEdit(&dialog);
    editor->setLineWrapMode(QPlainTextEdit::NoWrap);
    editor->setPlainText(QString::fromUtf8(definition_.toJson()));
    layout->addWidget(editor);

    auto *fileButtons = new QHBoxLayout();
    auto *loadButton = new QPushButton(tr("JSON kargatu..."), &dialog);
    auto *saveButton = new QPushButton(tr("JSON gorde..."), &dialog);
    fileButtons->addWidget(loadButton);
    fileButtons->addWidget(saveButton);
    fileButtons->addStretch();
    layout->addLayout(fileButtons);

    auto *dialogButtons = new QDialogButtonBox(QDialogButtonBox::Cancel, &dialog);
    dialogButtons->button(QDialogButtonBox::Cancel)->setText(tr("Utzi"));
    auto *applyButton = dialogButtons->addButton(tr("Formatua aplikatu"), QDialogButtonBox::AcceptRole);
    layout->addWidget(dialogButtons);
    connect(loadButton, &QPushButton::clicked, &dialog, [this, editor, &dialog]() {
        const QString path = QFileDialog::getOpenFileName(&dialog, tr("JSON formatua ireki"), QString(), tr("JSON fitxategiak (*.json)"));
        if (path.isEmpty())
            return;
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly))
        {
            showBasqueMessage(&dialog, QMessageBox::Warning, tr("Errorea"), tr("Ezin izan da JSON fitxategia irakurri."));
            return;
        }
        editor->setPlainText(QString::fromUtf8(file.readAll()));
    });
    connect(saveButton, &QPushButton::clicked, &dialog, [this, editor, &dialog]() {
        const QString path = QFileDialog::getSaveFileName(&dialog, tr("JSON formatua gorde"), QString(), tr("JSON fitxategiak (*.json)"));
        if (path.isEmpty())
            return;
        QSaveFile file(path);
        if (!file.open(QIODevice::WriteOnly) || file.write(editor->toPlainText().toUtf8()) < 0 || !file.commit())
        {
            showBasqueMessage(&dialog, QMessageBox::Warning, tr("Errorea"), tr("Ezin izan da JSON fitxategia gorde."));
            return;
        }
        showBasqueMessage(&dialog, QMessageBox::Information, tr("JSON gorde"), tr("Formatua JSON fitxategian gorde da."));
    });

    TournamentDefinition editedDefinition;
    connect(applyButton, &QPushButton::clicked, &dialog, [this, editor, &dialog, &editedDefinition]() {
        QString error;
        if (!TournamentDefinition::fromJson(editor->toPlainText().toUtf8(), editedDefinition, error))
        {
            showBasqueMessage(&dialog, QMessageBox::Warning, tr("JSON baliogabea"), error);
            return;
        }
        dialog.accept();
    });
    connect(dialogButtons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    if (dialog.exec() != QDialog::Accepted)
        return;

    if (hasTournamentResults())
    {
        QMessageBox message(QMessageBox::Question, tr("Formatua aldatu"),
                            tr("Formatua aldatzeak jokalariak eta uneko emaitzak ezabatuko ditu. Jarraitu nahi duzu?"),
                            QMessageBox::NoButton, this);
        QAbstractButton *applyAndReset = message.addButton(tr("Aplikatu eta berrabiarazi"), QMessageBox::AcceptRole);
        message.addButton(tr("Utzi"), QMessageBox::RejectRole);
        message.exec();
        if (message.clickedButton() != applyAndReset)
            return;
    }
    applyDefinition(editedDefinition);
    saveCheckpoint();
}

void Txapelketa::refreshTable(bool sortByRanking)
{
    Q_UNUSED(sortByRanking);
    std::vector<Player> ordered;
    if (activeRoundIndex_ >= 0 && activeRoundIndex_ < definition_.rounds.size())
        ordered = rankedEligiblePlayers(activeRoundIndex_);
    std::vector<int> eligibleIds = eligiblePlayerIds(activeRoundIndex_);
    std::vector<Player> remaining;
    for (const Player &player : players_)
    {
        if (std::find(eligibleIds.cbegin(), eligibleIds.cend(), player.id) == eligibleIds.cend())
            remaining.push_back(player);
    }
    std::stable_sort(remaining.begin(), remaining.end(), [](const Player &left, const Player &right) {
        return comparePlayerNames(left.name, right.name) < 0;
    });
    ordered.insert(ordered.end(), remaining.cbegin(), remaining.cend());
    table_->setPlayers(ordered, definition_.rounds, eligibleIds, activeRoundIndex_, !tournamentFinished_);
}

std::vector<Player> Txapelketa::rankedEligiblePlayers(int round) const
{
    std::vector<Player> ranking;
    for (int playerId : eligiblePlayerIds(round))
    {
        const Player *player = findPlayer(playerId);
        if (player)
            ranking.push_back(*player);
    }
    std::stable_sort(ranking.begin(), ranking.end(), [this, round](const Player &left, const Player &right) {
        const bool leftComplete = isResultComplete(left, round);
        const bool rightComplete = isResultComplete(right, round);
        if (leftComplete != rightComplete)
            return leftComplete;
        const int scoreOrder = compareScores(left, right, round);
        if (scoreOrder != 0)
            return scoreOrder < 0;
        return comparePlayerNames(left.name, right.name) < 0;
    });
    return ranking;
}

std::vector<int> Txapelketa::eligiblePlayerIds(int round) const
{
    std::vector<int> ids;
    if (round <= 0)
    {
        ids.reserve(players_.size());
        for (const Player &player : players_)
            ids.push_back(player.id);
        return ids;
    }
    if (round >= static_cast<int>(qualifiedIdsByRound_.size()))
        return ids;
    return qualifiedIdsByRound_[static_cast<std::size_t>(round)];
}

bool Txapelketa::isResultComplete(const Player &player, int round) const
{
    if (round < 0 || round >= definition_.rounds.size() || round >= static_cast<int>(player.results.size()))
        return false;
    const Result &result = player.results[static_cast<std::size_t>(round)];
    for (ResultCriterion criterion : definition_.rounds.at(round).criteria)
    {
        if (criterion == ResultCriterion::Time && !result.timeSeconds)
            return false;
        if (criterion == ResultCriterion::Errors && !result.errors)
            return false;
    }
    return true;
}

int Txapelketa::compareScores(const Player &left, const Player &right, int round) const
{
    if (round < 0 || round >= definition_.rounds.size())
        return 0;
    const RoundDefinition &roundDefinition = definition_.rounds.at(round);
    for (ResultCriterion criterion : roundDefinition.criteria)
    {
        qint64 leftValue = 0;
        qint64 rightValue = 0;
        bool leftHasValue = false;
        bool rightHasValue = false;
        const int firstRound = roundDefinition.rankingScope == RankingScope::Cumulative ? 0 : round;
        for (int resultRound = firstRound; resultRound <= round; ++resultRound)
        {
            if (resultRound >= definition_.rounds.size() || !roundHasCriterion(definition_.rounds.at(resultRound), criterion))
                continue;
            const Result *leftResult = resultRound < static_cast<int>(left.results.size()) ? &left.results[static_cast<std::size_t>(resultRound)] : nullptr;
            const Result *rightResult = resultRound < static_cast<int>(right.results.size()) ? &right.results[static_cast<std::size_t>(resultRound)] : nullptr;
            const std::optional<int> leftComponent = !leftResult ? std::nullopt :
                (criterion == ResultCriterion::Time ? leftResult->timeSeconds : leftResult->errors);
            const std::optional<int> rightComponent = !rightResult ? std::nullopt :
                (criterion == ResultCriterion::Time ? rightResult->timeSeconds : rightResult->errors);
            if (leftComponent)
            {
                leftValue += *leftComponent;
                leftHasValue = true;
            }
            if (rightComponent)
            {
                rightValue += *rightComponent;
                rightHasValue = true;
            }
        }
        if (leftHasValue != rightHasValue)
            return leftHasValue ? -1 : 1;
        if (!leftHasValue)
            continue;
        if (leftValue != rightValue)
            return leftValue < rightValue ? -1 : 1;
    }
    return 0;
}

bool Txapelketa::hasTournamentResults() const
{
    return !players_.empty();
}

void Txapelketa::applyDefinition(const TournamentDefinition &definition)
{
    definition_ = definition;
    players_.clear();
    qualifiedIdsByRound_.assign(static_cast<std::size_t>(definition_.rounds.size()), {});
    activeRoundIndex_ = 0;
    tournamentFinished_ = false;
    if (!definition_.rounds.isEmpty())
        timer_->configureRound(definition_.rounds.first().durationMinutes, definition_.rounds.first().name);
    updateRoundControls();
    refreshTable();
}

void Txapelketa::updateRoundControls()
{
    if (definition_.rounds.isEmpty())
        return;
    const QString roundName = activeRoundIndex_ >= 0 && activeRoundIndex_ < definition_.rounds.size()
                                  ? definition_.rounds.at(activeRoundIndex_).name : QString();
    currentRoundLabel_->setText(tr("%1 — %2").arg(definition_.name, roundName));
    if (tournamentFinished_)
        finishRoundButton_->setText(tr("Txapelketa amaituta"));
    else if (activeRoundIndex_ == definition_.rounds.size() - 1)
        finishRoundButton_->setText(tr("Txapelketa amaitu"));
    else
        finishRoundButton_->setText(tr("Txanda itxi eta hurrengora pasa"));
    finishRoundButton_->setEnabled(!tournamentFinished_);
    const bool canAddPlayers = !tournamentFinished_ && activeRoundIndex_ == 0;
    addPlayerButton_->setEnabled(canAddPlayers);
    playerNameInput_->setEnabled(canAddPlayers);
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
    if (!checkpointFailureReported_)
    {
        checkpointFailureReported_ = true;
        showBasqueMessage(this, QMessageBox::Warning, tr("Kopia automatikoa"),
                          tr("Ezin izan da azken emaitzen kopia gorde. Gorde emaitzak JSON fitxategi batean."));
    }
}
