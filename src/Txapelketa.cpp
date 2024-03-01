#include "Txapelketa.h"

static QString int2Time(int time)
{
    int hours = time / 3600;
    int minutes = (time / 60) % 60;
    int seconds = time % 60;

    return QString("%1:%2:%3").arg(hours, 2, 10, QChar('0')).arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));
}

static int time2Int(QString str)
{
    return str.mid(0, 2).toInt() * 3600 + str.mid(3, 2).toInt() * 60 + str.mid(6, 2).toInt();
}

Txapelketa::Txapelketa(QWidget *parent)
    : QWidget{parent}
{
    taula = new Taula(this);
    timer = new CountdownWidget(this);

    /* Public window */
    // publicwindow = new PublicWindow();
    // publicwindow->show();

    // Connect timer and public window
    // timer->public_label = publicwindow->timerlabel;

    /* Radio buttons */
    GameSelectorGroup = new QButtonGroup();
    GameSelectorLayout = new QHBoxLayout();
    GameSelectorLayout->setAlignment(Qt::AlignLeft);
    game_1 = new QRadioButton("Lehen Proba");
    game_2 = new QRadioButton("Finalaurrekoa");
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

    /* Buttons */
    addPlayer_BTN = new QPushButton("Jokalari berria");
    input_player_name = new QLineEdit();

    // /* image */
    //     // Creamos un QLabel para mostrar la imagen
    // image_label = new QLabel(this);
    // image_pixmap = new QPixmap("../img/jokoa1.jpeg");
    // if (!image_pixmap->isNull()) {
    //     image_label->setPixmap(*image_pixmap);
    //     image_label->resize(image_pixmap->size());
    // }

    /* Layouts */
    layout_h = new QHBoxLayout(this);
    layout_h->addWidget(addPlayer_BTN);
    layout_h->addWidget(input_player_name);

    // layout_table = new QHBoxLayout(this);
    // layout_table->addWidget(taula);
    // layout_table->addWidget(image_label);


    mainLayout->addWidget(timer);
    mainLayout->addLayout(GameSelectorLayout);
    mainLayout->addLayout(layout_h);
    mainLayout->addWidget(taula);

    setLayout(mainLayout);

    /* Connections */
    connect(updateTable_BTN, &QPushButton::clicked, this, &Txapelketa::updateTable);
    connect(addPlayer_BTN, &QPushButton::clicked, this, &Txapelketa::addPlayer);
}

void Txapelketa::deletePlayer(const QString &name)
{
}

void Txapelketa::addPlayer()
{
    QString name = input_player_name->text();
    if (name.isEmpty() || taula->checkPlayerExists(name))
    {
        return;
    }
    addPlayerNamed(name);
}

void Txapelketa::addPlayerNamed(const QString& name)
{
    Player p(name);
    QPushButton *setTime_BTN = new QPushButton("Berritu", this);
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

    qDebug() << "Looking for " << name ;

    for (auto row{0}; row < taula->rowCount(); ++row)
    {
        if (taula->item(row, PLAYER_NAME_COLUMN)->text() == name)
        {
            QTableWidgetItem *it;

            /* Time */
            if (!taula->item(row, timeColumn))
            {
                QString showData = int2Time(elapsedTime); 
                it = new QTableWidgetItem(showData);
                it->setData(Qt::DisplayRole, showData);
                it->setData(Qt::UserRole, elapsedTime);
                taula->setItem(row, timeColumn, it);
            }
            else
            {
                QString showData = int2Time(elapsedTime); 
                it = taula->item(row, timeColumn);
                it->setData(Qt::DisplayRole, showData);
                it->setData(Qt::UserRole, elapsedTime);
                taula->setItem(row, timeColumn, it);
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

        QTableWidgetItem *totErrItem = new QTableWidgetItem(total_errors);
        totErrItem->setData(Qt::EditRole, total_errors);
        taula->setItem(row, TOTAL_ERRORS_COLUMN, totErrItem);

        QString total_time_show_data = int2Time(total_time);
        QTableWidgetItem *totTimeItem = new QTableWidgetItem(total_time_show_data);
        totTimeItem->setData(Qt::DisplayRole, total_time_show_data); // Accepts a QVariant
        totTimeItem->setData(Qt::UserRole, total_time);
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