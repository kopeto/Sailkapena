#include "Taula.h"

#include "TimeFormat.h"

#include <QAbstractButton>
#include <QColor>
#include <QFont>
#include <QHeaderView>
#include <QFontMetrics>
#include <QHash>
#include <QKeyEvent>
#include <QMessageBox>
#include <QPalette>
#include <QPaintEvent>
#include <QPainter>
#include <QPushButton>
#include <QSignalBlocker>
#include <QStyle>
#include <QStyleOptionHeader>
#include <QStyledItemDelegate>
#include <QTableWidgetItem>

#include <algorithm>
#include <utility>

namespace
{
struct HeaderGroup
{
    int firstColumn{0};
    int lastColumn{0};
    QString title;
    QColor background;
};

class GroupedHeaderView final : public QHeaderView
{
public:
    explicit GroupedHeaderView(QWidget *parent = nullptr)
        : QHeaderView(Qt::Horizontal, parent)
    {
        setMinimumHeight(56);
        setDefaultAlignment(Qt::AlignCenter);
    }

    void setGroups(std::vector<HeaderGroup> groups)
    {
        groups_ = std::move(groups);
        viewport()->update();
    }

protected:
    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override
    {
        const bool grouped = std::any_of(groups_.cbegin(), groups_.cend(), [logicalIndex](const HeaderGroup &group) {
            return logicalIndex >= group.firstColumn && logicalIndex <= group.lastColumn;
        });
        if (!grouped || orientation() != Qt::Horizontal)
        {
            QHeaderView::paintSection(painter, rect, logicalIndex);
            return;
        }

        QRect metricRect = rect;
        metricRect.setTop(rect.top() + groupHeight_);
        QHeaderView::paintSection(painter, metricRect, logicalIndex);
    }

    void paintEvent(QPaintEvent *event) override
    {
        QHeaderView::paintEvent(event);
        if (orientation() != Qt::Horizontal)
            return;

        QPainter painter(viewport());
        for (const HeaderGroup &group : groups_)
        {
            if (isSectionHidden(group.firstColumn) || isSectionHidden(group.lastColumn))
                continue;
            const int left = sectionViewportPosition(group.firstColumn);
            const int right = sectionViewportPosition(group.lastColumn) + sectionSize(group.lastColumn);
            QRect groupRect(left, 0, right - left, groupHeight_);
            if (!groupRect.intersects(event->rect()))
                continue;

            QStyleOptionHeader option;
            option.initFrom(this);
            option.rect = groupRect;
            option.text = group.title;
            option.textAlignment = Qt::AlignCenter;
            option.orientation = Qt::Horizontal;
            option.section = group.firstColumn;
            option.position = QStyleOptionHeader::OnlyOneSection;
            option.palette.setColor(QPalette::Button, group.background);
            option.state |= QStyle::State_Enabled | QStyle::State_Raised;
            style()->drawControl(QStyle::CE_Header, &option, &painter, this);
        }
    }

private:
    static constexpr int groupHeight_{28};
    std::vector<HeaderGroup> groups_;
};

class RoundColumnDelegate final : public QStyledItemDelegate
{
public:
    explicit RoundColumnDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent)
    {
    }

    void setColumnBackgrounds(QHash<int, std::array<QColor, 2>> backgrounds)
    {
        backgrounds_ = std::move(backgrounds);
    }

    void setRowBackgrounds(std::array<QColor, 2> backgrounds)
    {
        rowBackgrounds_ = std::move(backgrounds);
    }

protected:
    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override
    {
        QStyledItemDelegate::initStyleOption(option, index);
        if (option->state & QStyle::State_Selected)
            return;

        const std::size_t rowShade = static_cast<std::size_t>(index.row() % 2);
        const auto background = backgrounds_.constFind(index.column());
        if (background != backgrounds_.cend())
            option->backgroundBrush = background.value()[static_cast<std::size_t>(index.row() % 2)];
        else
            option->backgroundBrush = rowBackgrounds_[rowShade];
    }

private:
    QHash<int, std::array<QColor, 2>> backgrounds_;
    std::array<QColor, 2> rowBackgrounds_{{QColor(255, 255, 255), QColor(235, 235, 235)}};
};

void setReadOnly(QTableWidgetItem *item)
{
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
}
}

Taula::Taula(QWidget *parent)
    : QTableWidget(parent)
{
    setHorizontalHeader(new GroupedHeaderView(this));
    setItemDelegate(new RoundColumnDelegate(this));
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setAlternatingRowColors(true);
    QPalette tablePalette = palette();
    tablePalette.setColor(QPalette::Base, QColor(255, 255, 255));
    tablePalette.setColor(QPalette::AlternateBase, QColor(232, 232, 232));
    tablePalette.setColor(QPalette::Highlight, QColor(190, 190, 190));
    tablePalette.setColor(QPalette::HighlightedText, QColor(25, 25, 25));
    setPalette(tablePalette);
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
    cutColumns_.assign(static_cast<std::size_t>(rounds.size()), std::array<int, 2>{{-1, -1}});
    std::vector<HeaderGroup> headerGroups;
    QHash<int, std::array<QColor, 2>> columnBackgrounds;

    QStringList headers{QString(), tr("Jokalaria")};
    int nextColumn = 2;
    for (qsizetype roundIndex = 0; roundIndex < rounds.size(); ++roundIndex)
    {
        const RoundDefinition &round = rounds.at(roundIndex);
        const int firstColumn = nextColumn;
        const QColor roundBackground = roundIndex % 2 == 0 ? QColor(224, 224, 224) : QColor(245, 245, 245);
        const std::array<QColor, 2> shadedRoundRows{{QColor(226, 226, 226), QColor(210, 210, 210)}};
        for (int field = 0; field < 2; ++field)
        {
            roundColumns_[static_cast<std::size_t>(roundIndex)][static_cast<std::size_t>(field)] = nextColumn++;
            headers.append(field == 0 ? criterionName(ResultCriterion::Time) : criterionName(ResultCriterion::Errors));
            if (roundIndex % 2 == 0)
                columnBackgrounds.insert(nextColumn - 1, shadedRoundRows);
        }
        headerGroups.push_back({firstColumn, nextColumn - 1, round.name, roundBackground});
        if (roundIndex + 1 < rounds.size() && round.advanceCount.has_value())
        {
            const int cutFirstColumn = nextColumn;
            cutColumns_[static_cast<std::size_t>(roundIndex)][0] = nextColumn++;
            headers.append(tr("Denbora metatua"));
            columnBackgrounds.insert(nextColumn - 1, {{QColor(244, 244, 244), QColor(228, 228, 228)}});
            cutColumns_[static_cast<std::size_t>(roundIndex)][1] = nextColumn++;
            headers.append(tr("Akatsak metatuak"));
            columnBackgrounds.insert(nextColumn - 1, {{QColor(244, 244, 244), QColor(228, 228, 228)}});
            headerGroups.push_back({cutFirstColumn, nextColumn - 1, QString(), QColor(240, 240, 240)});
        }
    }
    setColumnCount(nextColumn);
    setHorizontalHeaderLabels(headers);
    for (const HeaderGroup &group : headerGroups)
    {
        for (int column = group.firstColumn; column <= group.lastColumn; ++column)
            model()->setHeaderData(column, Qt::Horizontal, group.background, Qt::BackgroundRole);
    }
    auto *columnDelegate = static_cast<RoundColumnDelegate *>(itemDelegate());
    columnDelegate->setColumnBackgrounds(std::move(columnBackgrounds));
    columnDelegate->setRowBackgrounds({{QColor(255, 255, 255), QColor(235, 235, 235)}});
    static_cast<GroupedHeaderView *>(horizontalHeader())->setGroups(headerGroups);
    setColumnWidth(PLAYER_ACTION_COLUMN, 130);
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
        {
            QFont playerFont = nameItem->font();
            playerFont.setBold(true);
            nameItem->setFont(playerFont);
        }
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

        for (qsizetype cutRound = 0; cutRound < rounds.size(); ++cutRound)
        {
            for (int field = 0; field < 2; ++field)
            {
                const int column = cutColumns_[static_cast<std::size_t>(cutRound)][static_cast<std::size_t>(field)];
                if (column < 0)
                    continue;
                const ResultCriterion criterion = field == 0 ? ResultCriterion::Time : ResultCriterion::Errors;
                qint64 total = 0;
                bool complete = true;
                bool hasValue = false;
                for (int resultRound = 0; resultRound <= cutRound; ++resultRound)
                {
                    const RoundDefinition &round = rounds.at(resultRound);
                    if (resultRound >= static_cast<int>(player.results.size()))
                    {
                        complete = false;
                        break;
                    }
                    const Result &result = player.results[static_cast<std::size_t>(resultRound)];
                    const std::optional<int> value = field == 0 ? result.timeSeconds : result.errors;
                    if (!value)
                    {
                        const bool isScored = std::find(round.criteria.cbegin(), round.criteria.cend(), criterion) != round.criteria.cend();
                        if (isScored)
                        {
                            complete = false;
                            break;
                        }
                        continue;
                    }
                    total += *value;
                    hasValue = true;
                }
                auto *aggregateItem = new QTableWidgetItem();
                if (complete && hasValue)
                    aggregateItem->setText(field == 0 ? TimeFormat::format(total) : QString::number(total));
                setReadOnly(aggregateItem);
                setItem(row, column, aggregateItem);
            }
        }
    }

    horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    for (int column = 0; column < columnCount(); ++column)
        horizontalHeader()->setSectionResizeMode(column, QHeaderView::Interactive);
    setColumnWidth(PLAYER_ACTION_COLUMN, std::max(130, columnWidth(PLAYER_ACTION_COLUMN)));
    setColumnWidth(PLAYER_NAME_COLUMN, std::max(260, columnWidth(PLAYER_NAME_COLUMN)));
    const QFontMetrics headerFontMetrics(horizontalHeader()->font());
    for (const HeaderGroup &group : headerGroups)
    {
        if (group.title.isEmpty())
            continue;
        int groupWidth = 0;
        for (int column = group.firstColumn; column <= group.lastColumn; ++column)
            groupWidth += columnWidth(column);
        const int requiredWidth = headerFontMetrics.horizontalAdvance(group.title) + 28;
        if (groupWidth < requiredWidth)
            setColumnWidth(group.lastColumn, columnWidth(group.lastColumn) + requiredWidth - groupWidth);
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
