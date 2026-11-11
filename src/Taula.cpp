#include "Taula.h"

#include "TimeFormat.h"

#include <QAbstractButton>
#include <QApplication>
#include <QColor>
#include <QClipboard>
#include <QFont>
#include <QHeaderView>
#include <QFontMetrics>
#include <QHash>
#include <QIcon>
#include <QKeyEvent>
#include <QMessageBox>
#include <QMenu>
#include <QPalette>
#include <QPaintEvent>
#include <QPainter>
#include <QPixmap>
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

    void setColumnTones(QHash<int, int> tones)
    {
        columnTones_ = std::move(tones);
    }

protected:
    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override
    {
        QStyledItemDelegate::initStyleOption(option, index);
        if (option->state & QStyle::State_Selected)
            return;

        const int defaultTone = index.column() % 2 == 0 ? 248 : 240;
        const int columnTone = columnTones_.value(index.column(), defaultTone);
        const int rowAdjustment = index.row() % 2 == 0 ? 0 : 14;
        const int tone = std::max(0, columnTone - rowAdjustment);
        option->backgroundBrush = QColor(tone, tone, tone);
    }

private:
    QHash<int, int> columnTones_;
};

void setReadOnly(QTableWidgetItem *item)
{
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
}

QIcon clockIcon(const QColor &color)
{
    QPixmap pixmap(48, 48);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(color, 4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(QRect(6, 6, 36, 36));
    painter.drawLine(QPoint(24, 12), QPoint(24, 24));
    painter.drawLine(QPoint(24, 24), QPoint(33, 29));
    return QIcon(pixmap);
}
}

Taula::Taula(QWidget *parent)
    : QTableWidget(parent)
{
    setHorizontalHeader(new GroupedHeaderView(this));
    setItemDelegate(new RoundColumnDelegate(this));
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setAlternatingRowColors(true);
    QPalette tablePalette = palette();
    tablePalette.setColor(QPalette::Base, QColor(255, 255, 255));
    tablePalette.setColor(QPalette::AlternateBase, QColor(232, 232, 232));
    tablePalette.setColor(QPalette::Highlight, QColor(190, 190, 190));
    tablePalette.setColor(QPalette::HighlightedText, QColor(25, 25, 25));
    setPalette(tablePalette);
    horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    horizontalHeader()->setMinimumSectionSize(40);

    connect(this, &QWidget::customContextMenuRequested, this, [this](const QPoint &position) {
        if (publicDisplay_)
            return;

        QTableWidgetItem *nameItem = itemAt(position);
        if (!nameItem || nameItem->column() != PLAYER_NAME_COLUMN)
            return;

        const int playerId = nameItem->data(Qt::UserRole).toInt();
        const QString playerName = nameItem->text();
        QMenu menu(this);
        QAction *editNameAction = menu.addAction(tr("Izena editatu"));
        QAction *removePlayerAction = menu.addAction(tr("Jokalaria ezabatu"));
        menu.addSeparator();
        QAction *copyNameAction = menu.addAction(tr("Izena kopiatu"));
        QAction *chosenAction = menu.exec(viewport()->mapToGlobal(position));

        if (chosenAction == editNameAction)
            emit editPlayerNameRequested(playerId);
        else if (chosenAction == removePlayerAction)
        {
            QMessageBox message(QMessageBox::Question, tr("Jokalaria ezabatu"),
                                tr("%1 ezabatu nahi duzu?").arg(playerName), QMessageBox::NoButton, this);
            QAbstractButton *yesButton = message.addButton(tr("Bai"), QMessageBox::AcceptRole);
            message.addButton(tr("Ez"), QMessageBox::RejectRole);
            message.exec();
            if (message.clickedButton() == yesButton)
                emit removePlayerRequested(playerId);
        }
        else if (chosenAction == copyNameAction)
            QApplication::clipboard()->setText(playerName);
    });

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

void Taula::setPublicDisplay(bool publicDisplay)
{
    publicDisplay_ = publicDisplay;
    if (publicDisplay_)
        setEditTriggers(QAbstractItemView::NoEditTriggers);
    else
        setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    setSelectionMode(publicDisplay_ ? QAbstractItemView::NoSelection : QAbstractItemView::SingleSelection);
    setFocusPolicy(publicDisplay_ ? Qt::NoFocus : Qt::StrongFocus);
    setContextMenuPolicy(publicDisplay_ ? Qt::NoContextMenu : Qt::CustomContextMenu);
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
    QHash<int, int> columnTones;

    QStringList headers{QString(), tr("Jokalaria")};
    int nextColumn = 2;
    for (qsizetype roundIndex = 0; roundIndex < rounds.size(); ++roundIndex)
    {
        const RoundDefinition &round = rounds.at(roundIndex);
        const int firstColumn = nextColumn;
        const bool isFinalRound = roundIndex + 1 == rounds.size();
        const int roundTone = isFinalRound ? 216 : (roundIndex % 2 == 0 ? 228 : 244);
        for (ResultCriterion criterion : round.criteria)
        {
            const int field = criterion == ResultCriterion::Time ? 0 : 1;
            roundColumns_[static_cast<std::size_t>(roundIndex)][static_cast<std::size_t>(field)] = nextColumn++;
            headers.append(criterionName(criterion));
            columnTones.insert(nextColumn - 1, roundTone);
        }
        headerGroups.push_back({firstColumn, nextColumn - 1, round.name, QColor(roundTone, roundTone, roundTone)});
        if (roundIndex + 1 < rounds.size() && round.advanceCount.has_value())
        {
            const int cutFirstColumn = nextColumn;
            cutColumns_[static_cast<std::size_t>(roundIndex)][0] = nextColumn++;
            headers.append(tr("Denbora metatua"));
            columnTones.insert(nextColumn - 1, 236);
            cutColumns_[static_cast<std::size_t>(roundIndex)][1] = nextColumn++;
            headers.append(tr("Akatsak metatuak"));
            columnTones.insert(nextColumn - 1, 236);
            headerGroups.push_back({cutFirstColumn, nextColumn - 1, QString(), QColor(236, 236, 236)});
        }
    }
    setColumnCount(nextColumn);
    setHorizontalHeaderLabels(headers);
    model()->setHeaderData(PLAYER_ACTION_COLUMN, Qt::Horizontal, QColor(248, 248, 248), Qt::BackgroundRole);
    model()->setHeaderData(PLAYER_NAME_COLUMN, Qt::Horizontal, QColor(240, 240, 240), Qt::BackgroundRole);
    for (const HeaderGroup &group : headerGroups)
    {
        for (int column = group.firstColumn; column <= group.lastColumn; ++column)
            model()->setHeaderData(column, Qt::Horizontal, group.background, Qt::BackgroundRole);
    }
    for (auto tone = columnTones.cbegin(); tone != columnTones.cend(); ++tone)
        model()->setHeaderData(tone.key(), Qt::Horizontal, QColor(tone.value(), tone.value(), tone.value()), Qt::BackgroundRole);
    static_cast<RoundColumnDelegate *>(itemDelegate())->setColumnTones(std::move(columnTones));
    static_cast<GroupedHeaderView *>(horizontalHeader())->setGroups(headerGroups);
    setColumnWidth(PLAYER_ACTION_COLUMN, 44);
    setRowCount(static_cast<int>(players.size()));

    const bool activeRoundTracksTime = activeRound >= 0 && activeRound < rounds.size() &&
        std::find(rounds.at(activeRound).criteria.cbegin(), rounds.at(activeRound).criteria.cend(), ResultCriterion::Time) !=
            rounds.at(activeRound).criteria.cend();

    for (int row = 0; row < static_cast<int>(players.size()); ++row)
    {
        const Player &player = players[static_cast<std::size_t>(row)];
        const bool isEligible = std::find(eligibleIds.cbegin(), eligibleIds.cend(), player.id) != eligibleIds.cend();

        if (!publicDisplay_)
        {
            auto *recordButton = new QPushButton(this);
            recordButton->setIcon(clockIcon(palette().color(QPalette::ButtonText)));
            recordButton->setIconSize(QSize(22, 22));
            recordButton->setFixedSize(34, 30);
            recordButton->setAccessibleName(tr("Denbora erregistratu: %1").arg(player.name));
            recordButton->setAccessibleDescription(activeRoundTracksTime
                                                       ? tr("Kronometroaren denbora jokalari honentzat erregistratzen du.")
                                                       : tr("Txanda honetan ez da denborarik neurtzen."));
            recordButton->setToolTip(activeRoundTracksTime
                                         ? tr("Kronometroaren denbora erregistratu: %1").arg(player.name)
                                         : tr("Txanda honetan ez da denborarik neurtzen."));
            recordButton->setFocusPolicy(Qt::NoFocus);
            QPalette buttonPalette = recordButton->palette();
            const int buttonTone = row % 2 == 0 ? 248 : 234;
            buttonPalette.setColor(QPalette::Button, QColor(buttonTone, buttonTone, buttonTone));
            recordButton->setPalette(buttonPalette);
            recordButton->setVisible(true);
            recordButton->setEnabled(allowEditing && activeRoundTracksTime && isEligible);
            setCellWidget(row, PLAYER_ACTION_COLUMN, recordButton);
            connect(recordButton, &QPushButton::clicked, this, [this, id = player.id]() {
                emit recordTimeRequested(id);
            });
        }

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
                if (publicDisplay_)
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
                    const bool isScored = std::find(round.criteria.cbegin(), round.criteria.cend(), criterion) != round.criteria.cend();
                    if (!isScored)
                        continue;

                    if (resultRound >= static_cast<int>(player.results.size()))
                    {
                        complete = false;
                        continue;
                    }

                    const Result &result = player.results[static_cast<std::size_t>(resultRound)];
                    const std::optional<int> value = field == 0 ? result.timeSeconds : result.errors;
                    if (!value)
                    {
                        complete = false;
                        continue;
                    }

                    total += *value;
                    hasValue = true;
                }
                auto *aggregateItem = new QTableWidgetItem();
                if (hasValue)
                {
                    aggregateItem->setText(field == 0 ? TimeFormat::format(total) : QString::number(total));
                    if (!complete)
                        aggregateItem->setToolTip(tr("Emaitza batzuk falta dira; metatua sartutako balioekin kalkulatu da."));
                }
                setReadOnly(aggregateItem);
                setItem(row, column, aggregateItem);
            }
        }
    }

    horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    for (int column = 0; column < columnCount(); ++column)
        horizontalHeader()->setSectionResizeMode(column, QHeaderView::Interactive);
    setColumnWidth(PLAYER_ACTION_COLUMN, 44);
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
    setColumnHidden(PLAYER_ACTION_COLUMN, publicDisplay_);
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
