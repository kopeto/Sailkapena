#include "Taula.h"
#include "Txapelketa.h"




Taula::Taula(QWidget *parent) : QTableWidget(parent)
{
    QStringList headers;

    headers << " "
            << "zk"
            << " Jokalaria "
            << " Denbora1 "
            << " Akatsak1 "
            << " Denbora2 "
            << " Akatsak2 "
            << " Denbora3 "
            << " Akatsak3 "
            << " Denbora "
            << " Akatsak";
    setColumnCount(headers.size() + 1);
    setHorizontalHeaderLabels(headers);
}

void Taula::addPlayer(const Player &player)
{
    insertRow(rowCount());
    setItem(rowCount() - 1, PLAYER_ID_COLUMN, new QTableWidgetItem(QString("%1").arg(player.getId())));
    setItem(rowCount() - 1, PLAYER_NAME_COLUMN, new QTableWidgetItem(player.getName()));
}

void Taula::setTime()
{
    ;
}
