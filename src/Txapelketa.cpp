#include "Txapelketa.h"

Txapelketa::Txapelketa(QWidget *parent)
    : QWidget{parent}
{
    taula = new Taula(this);
    timer = new CountdownWidget(this);

    /* Radio buttons */
    GameSelectorGroup = new QButtonGroup();
    GameSelectorLayout = new QHBoxLayout();
    GameSelectorLayout->setAlignment(Qt::AlignLeft);
    game_1 = new QRadioButton("Lehen Jokoa");
    game_2 = new QRadioButton("Bigarren Jokoa");
    game_final = new QRadioButton("Finala");

    game_1->setChecked(true);
    game_2->setChecked(false);
    game_final->setChecked(false);

    GameSelectorLayout->addWidget(game_1);
    GameSelectorLayout->addWidget(game_2);
    GameSelectorLayout->addWidget(game_final);

    GameSelectorGroup->addButton(game_1);
    GameSelectorGroup->addButton(game_2);
    GameSelectorGroup->addButton(game_final);

    updateTable_BTN = new QPushButton("Taula berritu");
    GameSelectorLayout->addWidget(updateTable_BTN);

    mainLayout = new QVBoxLayout(this);

    mainLayout->addWidget(timer);
    mainLayout->addLayout(GameSelectorLayout);
    mainLayout->addWidget(taula);

    layout_h = new QHBoxLayout(this);

    /* Buttons */
    addPlayer_BTN = new QPushButton("Jokalari berria");
    input_player_name = new QLineEdit();

    /* Layouts */
    mainLayout->addLayout(layout_h);
    layout_h->addWidget(addPlayer_BTN);
    layout_h->addWidget(input_player_name);

    setLayout(mainLayout);

    /* Connections */
    connect(updateTable_BTN, &QPushButton::clicked, this, &Txapelketa::updateTable);
    connect(addPlayer_BTN, &QPushButton::clicked, this, &Txapelketa::addPlayer);
}

void Txapelketa::deletePlayer(const QString& name)
{

}

void Txapelketa::addPlayer()
{
    QString name = input_player_name->text();
    if (name.isEmpty() || taula->checkPlayerExists(name))
    {
        return;
    }

    Player p(input_player_name->text());
    QPushButton *setTime_BTN = new QPushButton("Denbora berritu", this);
    p.setTime_BTN = setTime_BTN;
    Players.push_back(p);
    taula->addPlayer(p);

    taula->setCellWidget(taula->rowCount() - 1, 0, setTime_BTN);

    connect(setTime_BTN, &QPushButton::clicked, this, std::bind(&Txapelketa::setTime, this, p.getName()));
}

void Txapelketa::printPlayers()
{
    for (const auto &p : Players)
    {
        qDebug() << p.getId() << ": " << p.getName();
        int i = 0;
        for (const auto &[t, e] : p.results)
        {
            qDebug() << "\tTxanda " << i << ":" << t << ":" << e;
        }
    }
}
void Txapelketa::setTime(const QString &name)
{
    /* Update results in player */
    /* Game 0 */
    int elapsedTime = timer->getElapsedTime();
    int errors = 0;

    int timeColumn{0};
    int errorsColumn{0};
    int game{0};

    std::tie(timeColumn, errorsColumn) = getCurrentGameColumns();

    for (auto row{0}; row < taula->rowCount(); ++row)
    {
        if (taula->item(row, PLAYER_NAME_COLUMN)->text() == name)
        {
            QTableWidgetItem *it;

            /* Time */
            if (!taula->item(row, timeColumn))
            {
                it = new QTableWidgetItem();
                it->setData(Qt::EditRole, elapsedTime);
                taula->setItem(row, timeColumn, it);
            }
            else
            {
                it = taula->item(row, timeColumn);
                it->setData(Qt::EditRole, elapsedTime);
            }

            /* Errors */
            if (!taula->item(row, errorsColumn))
            {
                errors = 0;
                it = new QTableWidgetItem();
                it->setData(Qt::EditRole, errors);
                taula->setItem(row, errorsColumn, it);
            }
            else
            {
                errors = taula->item(row, errorsColumn)->text().toInt();
            }
        }
    }
}

// void Txapelketa::setTime(const Player &player)
// {
//     setTime(player.getName());
// }

void Txapelketa::updateTable()
{
    updateInternals_();

    /* Sort */
    taula->sortByColumn(TOTAL_TIME_COLUMN, Qt::SortOrder::AscendingOrder);
    taula->sortByColumn(TOTAL_ERRORS_COLUMN, Qt::SortOrder::AscendingOrder);

    updateReals_();
}

void Txapelketa::updateInternals_()
{
    for (auto row{0}; row < taula->rowCount(); ++row)
    {
        int total_errors = taula->getRowTotalErrorsInternal(row);
        int total_time = taula->getRowTotalTimeInternal(row);

        QTableWidgetItem *totErrItem = new QTableWidgetItem();
        totErrItem->setData(Qt::EditRole, total_errors); // Accepts a QVariant
        taula->setItem(row, TOTAL_ERRORS_COLUMN, totErrItem);

        QTableWidgetItem *totTimeItem = new QTableWidgetItem();
        totTimeItem->setData(Qt::EditRole, total_time); // Accepts a QVariant
        taula->setItem(row, TOTAL_TIME_COLUMN, totTimeItem);
    }
}

void Txapelketa::updateReals_()
{
    for (auto row{0}; row < taula->rowCount(); ++row)
    {
        int total_errors = taula->getRowTotalErrorsReal(row);
        int total_time = taula->getRowTotalTimeReal(row);

        QTableWidgetItem *totErrItem = new QTableWidgetItem();
        totErrItem->setData(Qt::EditRole, total_errors); // Accepts a QVariant
        taula->setItem(row, TOTAL_ERRORS_COLUMN, totErrItem);

        QTableWidgetItem *totTimeItem = new QTableWidgetItem();
        totTimeItem->setData(Qt::EditRole, total_time); // Accepts a QVariant
        taula->setItem(row, TOTAL_TIME_COLUMN, totTimeItem);
    }
}

std::pair<int, int> Txapelketa::getCurrentGameColumns()
{
    std::pair<int, int> cols;

    if (game_1->isChecked())
    {
        cols.first = GAME1_TIME_COLUMN;
        cols.second = GAME1_ERRORS_COLUMN;
    }
    if (game_2->isChecked())
    {
        cols.first = GAME2_TIME_COLUMN;
        cols.second = GAME2_ERRORS_COLUMN;
    }
    if (game_final->isChecked())
    {
        cols.first = GAME3_TIME_COLUMN;
        cols.second = GAME3_ERRORS_COLUMN;
    }

    return cols;
}