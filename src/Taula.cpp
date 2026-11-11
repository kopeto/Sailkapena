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
#include <array>
#include <optional>

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
    setColumnCount(11);
    setHorizontalHeaderLabels({
        "", "Jokalaria", "1. proba: denbora", "1. proba: akatsak",
        "2. proba: denbora", "2. proba: akatsak", "Guztira: denbora",
        "Guztira: akatsak", "", "Finala: denbora", "Finala: akatsak"
    });

    setColumnWidth(PLAYER_ACTION_COLUMN, 130);
    setColumnWidth(PLAYER_NAME_COLUMN, 250);
    setColumnWidth(GAME1_ERRORS_COLUMN, 75);
    setColumnWidth(GAME2_ERRORS_COLUMN, 75);
    setColumnWidth(TOTAL_ERRORS_COLUMN, 75);
    setColumnWidth(FINALS_SEPARATOR_COLUMN, 2);
    horizontalHeader()->setSectionResizeMode(FINALS_SEPARATOR_COLUMN, QHeaderView::Fixed);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setAlternatingRowColors(true);

    connect(this, &QTableWidget::itemChanged, this, [this](QTableWidgetItem *changedItem) {
        const int column = changedItem->column();
        int game = -1;
        int field = -1;
        switch (column)
        {
        case GAME1_TIME_COLUMN: game = 0; field = 0; break;
        case GAME1_ERRORS_COLUMN: game = 0; field = 1; break;
        case GAME2_TIME_COLUMN: game = 1; field = 0; break;
        case GAME2_ERRORS_COLUMN: game = 1; field = 1; break;
        case FINALS_TIME_COLUMN: game = 2; field = 0; break;
        case FINALS_ERRORS_COLUMN: game = 2; field = 1; break;
        default: return;
        }

        QTableWidgetItem *nameItem = item(changedItem->row(), PLAYER_NAME_COLUMN);
        if (!nameItem)
            return;

        const int playerId = nameItem->data(Qt::UserRole).toInt();
        emit resultEdited(playerId, game, field, changedItem->text());
    });
}

void Taula::setPlayers(const std::vector<Player> &players, const std::vector<int> &finalistIds, bool finalMode)
{
    const QSignalBlocker blocker(this);
    setSortingEnabled(false);
    clearContents();
    setRowCount(static_cast<int>(players.size()));

    for (int row = 0; row < static_cast<int>(players.size()); ++row)
    {
        const Player &player = players[static_cast<std::size_t>(row)];
        const bool isFinalist = std::find(finalistIds.begin(), finalistIds.end(), player.id) != finalistIds.end();

        auto *recordButton = new QPushButton(tr("Denbora sartu"), this);
        recordButton->setFocusPolicy(Qt::NoFocus);
        if (finalMode && !isFinalist)
            recordButton->setEnabled(false);
        setCellWidget(row, PLAYER_ACTION_COLUMN, recordButton);
        connect(recordButton, &QPushButton::clicked, this, [this, id = player.id]() {
            emit recordTimeRequested(id);
        });

        auto *nameItem = new QTableWidgetItem(player.name);
        nameItem->setData(Qt::UserRole, player.id);
        setReadOnly(nameItem);
        setItem(row, PLAYER_NAME_COLUMN, nameItem);

        const std::array<int, 3> timeColumns{{GAME1_TIME_COLUMN, GAME2_TIME_COLUMN, FINALS_TIME_COLUMN}};
        const std::array<int, 3> errorColumns{{GAME1_ERRORS_COLUMN, GAME2_ERRORS_COLUMN, FINALS_ERRORS_COLUMN}};
        for (int game = 0; game < 3; ++game)
        {
            const Result &result = player.results[static_cast<std::size_t>(game)];
            const bool showResult = game != 2 || isFinalist;
            const bool canEditResult = game != 2 || finalMode;

            auto *timeItem = new QTableWidgetItem();
            if (showResult && result.timeSeconds)
            {
                timeItem->setText(TimeFormat::format(*result.timeSeconds));
                timeItem->setData(Qt::UserRole, *result.timeSeconds);
            }
            if (!showResult || !canEditResult)
                setReadOnly(timeItem);
            setItem(row, timeColumns[static_cast<std::size_t>(game)], timeItem);

            auto *errorsItem = new QTableWidgetItem();
            if (showResult && result.errors)
                errorsItem->setText(QString::number(*result.errors));
            if (!showResult || !canEditResult)
                setReadOnly(errorsItem);
            setItem(row, errorColumns[static_cast<std::size_t>(game)], errorsItem);
        }

        qint64 totalSeconds = 0;
        qint64 totalErrors = 0;
        bool hasPreliminaryTime = false;
        for (int game = 0; game < 2; ++game)
        {
            const Result &result = player.results[static_cast<std::size_t>(game)];
            if (result.timeSeconds)
            {
                totalSeconds += *result.timeSeconds;
                totalErrors += result.errors.value_or(0);
                hasPreliminaryTime = true;
            }
        }

        auto *totalTimeItem = new QTableWidgetItem();
        if (hasPreliminaryTime)
            totalTimeItem->setText(TimeFormat::format(totalSeconds));
        setReadOnly(totalTimeItem);
        setItem(row, TOTAL_TIME_COLUMN, totalTimeItem);

        auto *totalErrorsItem = new QTableWidgetItem();
        if (hasPreliminaryTime)
            totalErrorsItem->setText(QString::number(totalErrors));
        setReadOnly(totalErrorsItem);
        setItem(row, TOTAL_ERRORS_COLUMN, totalErrorsItem);

        if (isFinalist)
        {
            for (int column = PLAYER_NAME_COLUMN; column < columnCount(); ++column)
            {
                if (column != FINALS_SEPARATOR_COLUMN && item(row, column))
                    item(row, column)->setBackground(QColor(210, 255, 210));
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
