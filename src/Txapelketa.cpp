#include "Txapelketa.h"

#include "CountDownWidget.h"
#include "PublicWindow.h"
#include "Taula.h"
#include "TimeFormat.h"

#include <QAbstractButton>
#include <QCheckBox>
#include <QCollator>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QLocale>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSaveFile>
#include <QSignalBlocker>
#include <QStandardItemModel>
#include <QSpinBox>
#include <QTimer>
#include <QVBoxLayout>
#include <QUuid>

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

struct RoundEditorRow
{
    QString id;
    QGroupBox *card{nullptr};
    QLineEdit *name{nullptr};
    QSpinBox *duration{nullptr};
    QComboBox *criteria{nullptr};
    QComboBox *rankingScope{nullptr};
    QCheckBox *advanceAll{nullptr};
    QSpinBox *advanceCount{nullptr};
    QLabel *advanceLabel{nullptr};
    QComboBox *tiePolicy{nullptr};
    QPushButton *moveUp{nullptr};
    QPushButton *moveDown{nullptr};
    QPushButton *remove{nullptr};
};
}

Txapelketa::Txapelketa(QWidget *parent)
    : QWidget(parent), definition_(TournamentDefinition::defaultDefinition())
{
    table_ = new Taula(this);
    timer_ = new CountdownWidget(this);
    currentRoundLabel_ = new QLabel(this);
    currentRoundLabel_->setText(tr("Uneko txanda:"));
    currentRoundLabel_->setStyleSheet(QStringLiteral("font-size: 18px; font-weight: bold;"));
    roundSelector_ = new QComboBox(this);
    roundSelector_->setMinimumWidth(220);

    finishRoundButton_ = new QPushButton(this);
    updateTableButton_ = new QPushButton(tr("Eguneratu"), this);
    roundControlLayout_ = new QHBoxLayout();
    roundControlLayout_->setAlignment(Qt::AlignLeft);
    roundControlLayout_->addWidget(currentRoundLabel_);
    roundControlLayout_->addWidget(roundSelector_);
    roundControlLayout_->addWidget(finishRoundButton_);
    roundControlLayout_->addWidget(updateTableButton_);

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
    connect(roundSelector_, &QComboBox::currentIndexChanged, this, &Txapelketa::selectRound);
    connect(updateTableButton_, &QPushButton::clicked, this, &Txapelketa::updateTable);
    connect(addPlayerButton_, &QPushButton::clicked, this, &Txapelketa::addPlayer);
    connect(playerNameInput_, &QLineEdit::returnPressed, this, &Txapelketa::addPlayer);
    connect(timer_, &CountdownWidget::publicDisplayToggleRequested, this, &Txapelketa::togglePublicWindow);
    connect(table_, &Taula::recordTimeRequested, this, &Txapelketa::recordTime);
    connect(table_, &Taula::editPlayerNameRequested, this, &Txapelketa::editPlayerName);
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
        loadFromFile_(checkpointToRestore);
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

void Txapelketa::editPlayerName(int playerId)
{
    Player *player = findPlayer(playerId);
    if (!player)
        return;

    bool accepted = false;
    const QString editedName = QInputDialog::getText(this, tr("Jokalariaren izena editatu"),
                                                    tr("Jokalariaren izena:"), QLineEdit::Normal,
                                                    player->name, &accepted).trimmed();
    if (!accepted || editedName == player->name)
        return;
    if (editedName.isEmpty())
    {
        showBasqueMessage(this, QMessageBox::Information, tr("Jokalariaren izena editatu"),
                          tr("Idatzi jokalariaren izena."));
        return;
    }
    if (editedName.contains(QLatin1Char('\n')) || editedName.contains(QLatin1Char('\r')))
    {
        showBasqueMessage(this, QMessageBox::Information, tr("Jokalariaren izena editatu"),
                          tr("Izenak ezin du lerro-jauzirik izan."));
        return;
    }

    const auto duplicate = std::find_if(players_.cbegin(), players_.cend(), [playerId, &editedName](const Player &candidate) {
        return candidate.id != playerId && candidate.name.compare(editedName, Qt::CaseInsensitive) == 0;
    });
    if (duplicate != players_.cend())
    {
        showBasqueMessage(this, QMessageBox::Information, tr("Jokalariaren izena editatu"),
                          tr("Izen hori duen jokalaria badago jada."));
        return;
    }

    player->name = editedName;
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
    const auto found = std::find_if(players_.begin(), players_.end(), [playerId](const Player &player) {
        return player.id == playerId;
    });
    if (found == players_.end())
        return;
    if (!tournamentFinished_ && activeRoundIndex_ > 0)
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
    if (!player || round < 0 || round >= definition_.rounds.size() || round >= static_cast<int>(player->results.size()))
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
        refreshTable();
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
    dialog.resize(920, 720);
    auto *layout = new QVBoxLayout(&dialog);
    auto *formatName = new QLineEdit(definition_.name, &dialog);
    auto *nameLayout = new QHBoxLayout();
    nameLayout->addWidget(new QLabel(tr("Txapelketaren izena"), &dialog));
    nameLayout->addWidget(formatName, 1);
    layout->addLayout(nameLayout);

    auto *scrollArea = new QScrollArea(&dialog);
    scrollArea->setWidgetResizable(true);
    auto *roundsContainer = new QWidget(scrollArea);
    auto *roundsLayout = new QVBoxLayout(roundsContainer);
    roundsLayout->setAlignment(Qt::AlignTop);
    roundsLayout->addStretch(1);
    scrollArea->setWidget(roundsContainer);
    layout->addWidget(scrollArea, 1);

    std::vector<RoundEditorRow> rows;
    QString formatId = definition_.formatId;
    auto updateRoundControls = [&rows]() {
        for (std::size_t index = 0; index < rows.size(); ++index)
        {
            RoundEditorRow &row = rows[index];
            row.card->setTitle(QObject::tr("%1. txanda").arg(index + 1));
            row.moveUp->setEnabled(index > 0);
            row.moveDown->setEnabled(index + 1 < rows.size());
            const bool isFinalRound = index + 1 == rows.size();
            row.advanceLabel->setText(isFinalRound ? QObject::tr("Aurrerapena: azken txanda")
                                                   : QObject::tr("Aurrera pasatzeko kopurua"));
            row.advanceAll->setEnabled(!isFinalRound);
            row.advanceCount->setEnabled(!isFinalRound && !row.advanceAll->isChecked());
            row.tiePolicy->setEnabled(!isFinalRound);
        }
    };

    auto createRoundRow = [&rows, roundsLayout, &updateRoundControls, &dialog](const RoundDefinition *source) {
        RoundEditorRow row;
        row.id = source ? source->id : QUuid::createUuid().toString(QUuid::WithoutBraces);
        row.card = new QGroupBox(roundsLayout->parentWidget());
        auto *grid = new QGridLayout(row.card);
        row.name = new QLineEdit(row.card);
        row.name->setPlaceholderText(QObject::tr("Adibidez: Finala"));
        row.duration = new QSpinBox(row.card);
        row.duration->setRange(1, 1440);
        row.duration->setSuffix(QObject::tr(" min"));
        row.criteria = new QComboBox(row.card);
        row.criteria->addItem(QObject::tr("Akatsak, ondoren denbora"), QStringLiteral("errors,time"));
        row.criteria->addItem(QObject::tr("Denbora, ondoren akatsak"), QStringLiteral("time,errors"));
        row.criteria->addItem(QObject::tr("Akatsak bakarrik"), QStringLiteral("errors"));
        row.criteria->addItem(QObject::tr("Denbora bakarrik"), QStringLiteral("time"));
        row.rankingScope = new QComboBox(row.card);
        row.rankingScope->addItem(QObject::tr("Txanda honetako emaitzak"), QStringLiteral("round"));
        row.rankingScope->addItem(QObject::tr("Emaitza metatuak"), QStringLiteral("cumulative"));
        row.advanceLabel = new QLabel(row.card);
        row.advanceAll = new QCheckBox(QObject::tr("Denak pasatuko dira"), row.card);
        row.advanceCount = new QSpinBox(row.card);
        row.advanceCount->setRange(1, 1000000);
        row.advanceCount->setValue(6);
        row.tiePolicy = new QComboBox(row.card);
        row.tiePolicy->addItem(QObject::tr("Berdindutako guztiak pasatu"), QStringLiteral("includeTies"));
        row.tiePolicy->addItem(QObject::tr("Eskuz aukeratu"), QStringLiteral("manual"));
        row.moveUp = new QPushButton(QObject::tr("Gora"), row.card);
        row.moveDown = new QPushButton(QObject::tr("Behera"), row.card);
        row.remove = new QPushButton(QObject::tr("Txanda kendu"), row.card);

        grid->addWidget(new QLabel(QObject::tr("Izena"), row.card), 0, 0);
        grid->addWidget(row.name, 0, 1);
        grid->addWidget(new QLabel(QObject::tr("Iraupena"), row.card), 0, 2);
        grid->addWidget(row.duration, 0, 3);
        grid->addWidget(new QLabel(QObject::tr("Irizpideak eta lehentasuna"), row.card), 1, 0);
        grid->addWidget(row.criteria, 1, 1, 1, 3);
        grid->addWidget(new QLabel(QObject::tr("Sailkapena"), row.card), 2, 0);
        grid->addWidget(row.rankingScope, 2, 1, 1, 3);
        grid->addWidget(row.advanceLabel, 3, 0);
        grid->addWidget(row.advanceAll, 3, 1);
        grid->addWidget(row.advanceCount, 3, 2);
        grid->addWidget(new QLabel(QObject::tr("Berdinketak"), row.card), 4, 0);
        grid->addWidget(row.tiePolicy, 4, 1, 1, 3);
        auto *orderButtons = new QHBoxLayout();
        orderButtons->addStretch();
        orderButtons->addWidget(row.moveUp);
        orderButtons->addWidget(row.moveDown);
        orderButtons->addWidget(row.remove);
        grid->addLayout(orderButtons, 5, 0, 1, 4);

        if (source)
        {
            row.name->setText(source->name);
            row.duration->setValue(source->durationMinutes);
            QStringList criteriaKeys;
            for (ResultCriterion criterion : source->criteria)
                criteriaKeys.append(criterion == ResultCriterion::Time ? QStringLiteral("time") : QStringLiteral("errors"));
            row.criteria->setCurrentIndex(row.criteria->findData(criteriaKeys.join(QLatin1Char(','))));
            row.rankingScope->setCurrentIndex(row.rankingScope->findData(
                source->rankingScope == RankingScope::Cumulative ? QStringLiteral("cumulative") : QStringLiteral("round")));
            row.tiePolicy->setCurrentIndex(row.tiePolicy->findData(
                source->tiePolicy == TiePolicy::Manual ? QStringLiteral("manual") : QStringLiteral("includeTies")));
            row.advanceAll->setChecked(!source->advanceCount.has_value());
            row.advanceCount->setValue(source->advanceCount.value_or(6));
        }

        QWidget *card = row.card;
        rows.push_back(row);
        roundsLayout->insertWidget(static_cast<int>(rows.size() - 1), card);
        QObject::connect(row.advanceAll, &QCheckBox::toggled, card, [&updateRoundControls](bool) {
            updateRoundControls();
        });
        QObject::connect(row.moveUp, &QPushButton::clicked, card, [card, &rows, roundsLayout, &updateRoundControls]() {
            const auto found = std::find_if(rows.begin(), rows.end(), [card](const RoundEditorRow &candidate) {
                return candidate.card == card;
            });
            if (found == rows.begin() || found == rows.end())
                return;
            const std::size_t index = static_cast<std::size_t>(std::distance(rows.begin(), found));
            roundsLayout->removeWidget(card);
            std::swap(rows[index], rows[index - 1]);
            roundsLayout->insertWidget(static_cast<int>(index - 1), card);
            updateRoundControls();
        });
        QObject::connect(row.moveDown, &QPushButton::clicked, card, [card, &rows, roundsLayout, &updateRoundControls]() {
            const auto found = std::find_if(rows.begin(), rows.end(), [card](const RoundEditorRow &candidate) {
                return candidate.card == card;
            });
            if (found == rows.end() || std::next(found) == rows.end())
                return;
            const std::size_t index = static_cast<std::size_t>(std::distance(rows.begin(), found));
            roundsLayout->removeWidget(card);
            std::swap(rows[index], rows[index + 1]);
            roundsLayout->insertWidget(static_cast<int>(index + 1), card);
            updateRoundControls();
        });
        QObject::connect(row.remove, &QPushButton::clicked, card, [card, &rows, roundsLayout, &updateRoundControls, &dialog]() {
            if (rows.size() <= 1)
            {
                showBasqueMessage(&dialog, QMessageBox::Information, QObject::tr("Txandak"),
                                  QObject::tr("Gutxienez txanda bat eduki behar du txapelketak."));
                return;
            }
            roundsLayout->removeWidget(card);
            rows.erase(std::remove_if(rows.begin(), rows.end(), [card](const RoundEditorRow &candidate) {
                return candidate.card == card;
            }), rows.end());
            card->deleteLater();
            updateRoundControls();
        });
        updateRoundControls();
    };

    auto replaceRounds = [&rows, roundsLayout]() {
        while (!rows.empty())
        {
            roundsLayout->removeWidget(rows.back().card);
            delete rows.back().card;
            rows.pop_back();
        }
    };

    for (const RoundDefinition &round : definition_.rounds)
        createRoundRow(&round);

    auto *roundButtons = new QHBoxLayout();
    auto *addRoundButton = new QPushButton(tr("Gehitu txanda"), &dialog);
    auto *importButton = new QPushButton(tr("JSON inportatu..."), &dialog);
    auto *exportButton = new QPushButton(tr("JSON esportatu..."), &dialog);
    roundButtons->addWidget(addRoundButton);
    roundButtons->addStretch();
    roundButtons->addWidget(importButton);
    roundButtons->addWidget(exportButton);
    layout->addLayout(roundButtons);

    auto *dialogButtons = new QDialogButtonBox(QDialogButtonBox::Cancel, &dialog);
    dialogButtons->button(QDialogButtonBox::Cancel)->setText(tr("Utzi"));
    auto *applyButton = dialogButtons->addButton(tr("Formatua aplikatu"), QDialogButtonBox::AcceptRole);
    layout->addWidget(dialogButtons);

    auto collectDefinition = [&rows, formatName, &formatId](TournamentDefinition &result, QString &error) {
        if (rows.empty())
        {
            error = QObject::tr("Gutxienez txanda bat eduki behar du txapelketak.");
            return false;
        }
        TournamentDefinition draft;
        draft.formatId = formatId.trimmed().isEmpty() ? QStringLiteral("custom-tournament") : formatId;
        draft.name = formatName->text().trimmed().isEmpty() ? QObject::tr("Txapelketa") : formatName->text().trimmed();
        for (std::size_t index = 0; index < rows.size(); ++index)
        {
            const RoundEditorRow &source = rows[index];
            RoundDefinition round;
            round.id = source.id;
            round.name = source.name->text().trimmed().isEmpty()
                             ? QObject::tr("%1. txanda").arg(index + 1) : source.name->text().trimmed();
            round.durationMinutes = source.duration->value();
            const QStringList criteria = source.criteria->currentData().toString().split(QLatin1Char(','));
            for (const QString &criterion : criteria)
                round.criteria.append(criterion == QStringLiteral("time") ? ResultCriterion::Time : ResultCriterion::Errors);
            round.rankingScope = source.rankingScope->currentData().toString() == QStringLiteral("cumulative")
                                     ? RankingScope::Cumulative : RankingScope::Round;
            round.tiePolicy = source.tiePolicy->currentData().toString() == QStringLiteral("manual")
                                  ? TiePolicy::Manual : TiePolicy::IncludeTies;
            if (index + 1 < rows.size())
                round.advanceCount = source.advanceAll->isChecked() ? std::nullopt
                                                                    : std::optional<int>(source.advanceCount->value());
            draft.rounds.append(round);
        }
        return TournamentDefinition::fromJson(draft.toJson(), result, error);
    };

    connect(addRoundButton, &QPushButton::clicked, &dialog, [createRoundRow]() mutable {
        createRoundRow(nullptr);
    });
    connect(importButton, &QPushButton::clicked, &dialog,
            [this, &dialog, &formatName, &formatId, &replaceRounds, &createRoundRow]() {
        const QString path = QFileDialog::getOpenFileName(&dialog, tr("JSON formatua inportatu"), QString(),
                                                          tr("JSON fitxategiak (*.json)"));
        if (path.isEmpty())
            return;
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly))
        {
            showBasqueMessage(&dialog, QMessageBox::Warning, tr("Errorea"), tr("Ezin izan da JSON fitxategia irakurri."));
            return;
        }
        TournamentDefinition imported;
        QString error;
        if (!TournamentDefinition::fromJson(file.readAll(), imported, error))
        {
            showBasqueMessage(&dialog, QMessageBox::Warning, tr("JSON baliogabea"), error);
            return;
        }
        formatId = imported.formatId;
        formatName->setText(imported.name);
        replaceRounds();
        for (const RoundDefinition &round : imported.rounds)
            createRoundRow(&round);
    });
    connect(exportButton, &QPushButton::clicked, &dialog, [this, &dialog, &collectDefinition]() {
        TournamentDefinition exported;
        QString error;
        if (!collectDefinition(exported, error))
        {
            showBasqueMessage(&dialog, QMessageBox::Warning, tr("Formatu baliogabea"), error);
            return;
        }
        const QString path = QFileDialog::getSaveFileName(&dialog, tr("JSON formatua esportatu"), QString(),
                                                          tr("JSON fitxategiak (*.json)"));
        if (path.isEmpty())
            return;
        QSaveFile file(path);
        const QByteArray json = exported.toJson();
        if (!file.open(QIODevice::WriteOnly) || file.write(json) != json.size() || !file.commit())
        {
            showBasqueMessage(&dialog, QMessageBox::Warning, tr("Errorea"), tr("Ezin izan da JSON fitxategia gorde."));
            return;
        }
        showBasqueMessage(&dialog, QMessageBox::Information, tr("JSON esportatu"),
                          tr("Txapelketaren formatua JSON fitxategian esportatu da."));
    });

    TournamentDefinition editedDefinition;
    connect(applyButton, &QPushButton::clicked, &dialog, [this, &dialog, &collectDefinition, &editedDefinition]() {
        QString error;
        if (!collectDefinition(editedDefinition, error))
        {
            showBasqueMessage(&dialog, QMessageBox::Warning, tr("Formatu baliogabea"), error);
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
    if (publicWindow_)
        publicWindow_->standingsTable()->setPlayers(ordered, definition_.rounds, eligibleIds, activeRoundIndex_, false);
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
    currentRoundLabel_->setText(tr("Uneko txanda:"));
    const QSignalBlocker blocker(roundSelector_);
    roundSelector_->clear();
    for (qsizetype index = 0; index < definition_.rounds.size(); ++index)
    {
        roundSelector_->addItem(tr("%1. %2").arg(index + 1).arg(definition_.rounds.at(index).name),
                                static_cast<int>(index));
    }
    if (auto *model = qobject_cast<QStandardItemModel *>(roundSelector_->model()))
    {
        for (int index = 0; index < roundSelector_->count(); ++index)
        {
            if (QStandardItem *item = model->item(index))
                item->setFlags(index <= activeRoundIndex_ + 1 ? Qt::ItemIsEnabled | Qt::ItemIsSelectable : Qt::NoItemFlags);
        }
    }
    roundSelector_->setCurrentIndex(activeRoundIndex_);
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

void Txapelketa::selectRound(int roundIndex)
{
    if (roundIndex < 0 || roundIndex >= definition_.rounds.size())
        return;
    if (roundIndex == activeRoundIndex_)
        return;
    if (roundIndex == activeRoundIndex_ + 1)
    {
        finishCurrentRound();
        updateRoundControls();
        return;
    }
    if (roundIndex > activeRoundIndex_)
    {
        updateRoundControls();
        return;
    }

    activeRoundIndex_ = roundIndex;
    tournamentFinished_ = false;
    timer_->configureRound(definition_.rounds.at(activeRoundIndex_).durationMinutes,
                           definition_.rounds.at(activeRoundIndex_).name);
    updateRoundControls();
    refreshTable();
    saveCheckpoint();
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
