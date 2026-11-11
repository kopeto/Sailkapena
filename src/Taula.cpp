#include "Taula.h"

#include "TimeFormat.h"

#include <QAbstractButton>
#include <QColor>
#include <QHeaderView>
#include <QKeyEvent>
#include <QMessageBox>
#include <QPushButton>
#include <QSignalBlocker>
#include <QTableWidgetItem>

#include <algorithm>

namespace
{
void setReadOnly(QTableWidgetItem *item)
{
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
}
}

Taula::Taula(QWidget *parent)
    : QTableWidget(parent)
{
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setAlternatingRowColors(true);
    horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    horizontalHeader()->setMinimumSectionSize(90);

    connect(this, &QTableWidget::itemChanged, this, [this](QTableWidgetItem *changedItem) {
        if (changedItem->column() < PLAYER_NAME_COLUMN)
            return;
        QTableWidgetItem *nameItem = item(changedItem->row(), PLAYER_NAME_COLUMN);
        if (!nameItem)
            return;
        const int playerId = nameItem->data(Qt::UserRole).toInt();

        for (std::size_t round = 0; round < roundColumns_.size(); ++round)
        {
            for (int field = 0; field < 2; ++field)
            {
                if (roundColumns_[round][static_cast<std::size_t>(field)] == changedItem->column())
                {
                    emit resultEdited(playerId, static_cast<int>(round), field, changedItem->text());
                    return;
                }
            }
        }
    });
}

void Taula::setPlayers(const std::vector<Player> &players,
                       const QVector<RoundDefinition> &rounds,
                       const std::vector<int> &eligibleIds,
                       int activeRound,
                       bool allowEditing)
{
    const QSignalBlocker blocker(this);
    setSortingEnabled(false);
    clearContents();
    roundColumns_.assign(static_cast<std::size_t>(rounds.size()), std::array<int, 2>{{-1, -1}});
    aggregateColumns_ = {{-1, -1}};

    QStringList headers{QString(), tr("Jokalaria")};
    int nextColumn = 2;
    for (qsizetype roundIndex = 0; roundIndex < rounds.size(); ++roundIndex)
    {
        const RoundDefinition &round = rounds.at(roundIndex);
        for (ResultCriterion criterion : round.criteria)
        {
            const int field = criterion == ResultCriterion::Time ? 0 : 1;
            roundColumns_[static_cast<std::size_t>(roundIndex)][static_cast<std::size_t>(field)] = nextColumn++;
            headers.append(tr("%1: %2").arg(round.name, criterionName(criterion)));
        }
    }
    if (activeRound >= 0 && activeRound < rounds.size() &&
        rounds.at(activeRound).rankingScope == RankingScope::Cumulative)
    {
        for (ResultCriterion criterion : rounds.at(activeRound).criteria)
        {
            const int field = criterion == ResultCriterion::Time ? 0 : 1;
            aggregateColumns_[static_cast<std::size_t>(field)] = nextColumn++;
            headers.append(tr("Metatua: %1").arg(criterionName(criterion)));
        }
    }
    setColumnCount(nextColumn);
    setHorizontalHeaderLabels(headers);
    setColumnWidth(PLAYER_ACTION_COLUMN, 130);
    horizontalHeader()->setSectionResizeMode(PLAYER_NAME_COLUMN, QHeaderView::Interactive);
    setColumnWidth(PLAYER_NAME_COLUMN, 260);
    setColumnHidden(PLAYER_ACTION_COLUMN, !allowEditing || activeRound < 0 || activeRound >= rounds.size() ||
                    std::find(rounds.at(activeRound).criteria.cbegin(), rounds.at(activeRound).criteria.cend(), ResultCriterion::Time) ==
                        rounds.at(activeRound).criteria.cend());
    setRowCount(static_cast<int>(players.size()));

    const bool activeRoundTracksTime = activeRound >= 0 && activeRound < rounds.size() &&
        std::find(rounds.at(activeRound).criteria.cbegin(), rounds.at(activeRound).criteria.cend(), ResultCriterion::Time) !=
            rounds.at(activeRound).criteria.cend();

    for (int row = 0; row < static_cast<int>(players.size()); ++row)
    {
        const Player &player = players[static_cast<std::size_t>(row)];
        const bool isEligible = std::find(eligibleIds.cbegin(), eligibleIds.cend(), player.id) != eligibleIds.cend();

        auto *recordButton = new QPushButton(tr("Denbora sartu"), this);
        recordButton->setFocusPolicy(Qt::NoFocus);
        recordButton->setVisible(allowEditing && activeRoundTracksTime);
        recordButton->setEnabled(allowEditing && activeRoundTracksTime && isEligible);
        setCellWidget(row, PLAYER_ACTION_COLUMN, recordButton);
        connect(recordButton, &QPushButton::clicked, this, [this, id = player.id]() {
            emit recordTimeRequested(id);
        });

        auto *nameItem = new QTableWidgetItem(player.name);
        nameItem->setData(Qt::UserRole, player.id);
        setReadOnly(nameItem);
        if (activeRound >= 0 && isEligible)
            nameItem->setBackground(QColor(218, 245, 218));
        setItem(row, PLAYER_NAME_COLUMN, nameItem);

        for (qsizetype roundIndex = 0; roundIndex < rounds.size(); ++roundIndex)
        {
            const bool isEditableRound = allowEditing && static_cast<int>(roundIndex) == activeRound && isEligible;
            const Result emptyResult;
            const Result &result = roundIndex < static_cast<qsizetype>(player.results.size())
                                       ? player.results[static_cast<std::size_t>(roundIndex)] : emptyResult;
            for (int field = 0; field < 2; ++field)
            {
                const int column = roundColumns_[static_cast<std::size_t>(roundIndex)][static_cast<std::size_t>(field)];
                if (column < 0)
                    continue;

                auto *resultItem = new QTableWidgetItem();
                if (field == 0 && result.timeSeconds)
                {
                    resultItem->setText(TimeFormat::format(*result.timeSeconds));
                    resultItem->setData(Qt::UserRole, *result.timeSeconds);
                }
                else if (field == 1 && result.errors)
                {
                    resultItem->setText(QString::number(*result.errors));
                }
                if (!isEditableRound)
                    setReadOnly(resultItem);
                setItem(row, column, resultItem);
            }
        }

        if (activeRound >= 0 && activeRound < rounds.size() &&
            rounds.at(activeRound).rankingScope == RankingScope::Cumulative)
        {
            for (int field = 0; field < 2; ++field)
            {
                const int column = aggregateColumns_[static_cast<std::size_t>(field)];
                if (column < 0)
                    continue;
                const ResultCriterion criterion = field == 0 ? ResultCriterion::Time : ResultCriterion::Errors;
                qint64 total = 0;
                bool complete = true;
                for (int resultRound = 0; resultRound <= activeRound; ++resultRound)
                {
                    const RoundDefinition &round = rounds.at(resultRound);
                    if (std::find(round.criteria.cbegin(), round.criteria.cend(), criterion) == round.criteria.cend())
                        continue;
                    if (resultRound >= static_cast<int>(player.results.size()))
                    {
                        complete = false;
                        break;
                    }
                    const Result &result = player.results[static_cast<std::size_t>(resultRound)];
                    const std::optional<int> value = field == 0 ? result.timeSeconds : result.errors;
                    if (!value)
                    {
                        complete = false;
                        break;
                    }
                    total += *value;
                }
                auto *aggregateItem = new QTableWidgetItem();
                if (complete)
                    aggregateItem->setText(field == 0 ? TimeFormat::format(total) : QString::number(total));
                setReadOnly(aggregateItem);
                setItem(row, column, aggregateItem);
            }
        }
    }
}

void Taula::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete)
    {
        const int row = currentRow();
        QTableWidgetItem *nameItem = row >= 0 ? item(row, PLAYER_NAME_COLUMN) : nullptr;
        if (!nameItem)
            return;

        const QString question = tr("%1 ezabatu nahi duzu?").arg(nameItem->text());
        QMessageBox message(QMessageBox::Question, tr("Jokalaria ezabatu"), question, QMessageBox::NoButton, this);
        QAbstractButton *yesButton = message.addButton(tr("Bai"), QMessageBox::AcceptRole);
        message.addButton(tr("Ez"), QMessageBox::RejectRole);
        message.exec();
        if (message.clickedButton() == yesButton)
            emit removePlayerRequested(nameItem->data(Qt::UserRole).toInt());
        return;
    }

    QTableWidget::keyPressEvent(event);
}
