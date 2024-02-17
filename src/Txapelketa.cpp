#include "Txapelketa.h"

Txapelketa::Txapelketa(QWidget *parent)
    : QWidget{parent}
{
    taula = new Taula(this);
    timer = new CountdownWidget(this);

    mainLayout = new QVBoxLayout(this);

    mainLayout->addWidget(timer);
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
    connect(addPlayer_BTN, &QPushButton::clicked, this, &Txapelketa::addPlayer);
}


void Txapelketa::addPlayer()
{
    if (input_player_name->text().isEmpty())
    {
        return;
    }

    Player p(input_player_name->text());
    Players.push_back(p);
    taula->addPlayer(p);

    QPushButton *btn_edit = new QPushButton("Set Time", this);
    taula->setCellWidget(taula->rowCount() - 1, 0, btn_edit);

    connect(btn_edit, &QPushButton::clicked, this, std::bind(&Txapelketa::setTime, this, p.getName()));
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

void Txapelketa::setTime(const QString& player_name)
{
    int elapsedTime = timer->getElapsedTime();

    qDebug() << "Elapsed : " << elapsedTime << " seconds for " << player_name;

    for(auto row{0}; row < taula->rowCount(); ++row)
    {
        if (taula->item(row, PLAYER_NAME_COLUMN)->text() == player_name)
        {
            taula->setItem(row, 3, new QTableWidgetItem(QString("%1").arg(elapsedTime)));
        }
    }
}

void Txapelketa::updateTable()
{
    ;
}