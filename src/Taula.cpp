#include "Taula.h"
#include "Txapelketa.h"

#include <QMessageBox>

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
            << " Akatsak ";
    setColumnCount(headers.size());
    setHorizontalHeaderLabels(headers);

}

void Taula::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete)
    {
        // Obtener la fila seleccionada
        int row = currentRow();
        deleteRow(row);
    }
    else
    {
        // Llamar a la implementación base para otros casos de teclas
        QTableWidget::keyPressEvent(event);
    }
}

void Taula::deleteRow(int row)
{
    // Verificar si hay una fila seleccionada
    if (row >= 0 && row < rowCount())
    {
        QMessageBox::StandardButton reply;
        QString playername = getPlayerNameFromRow(row);
        QString galdera = "Seguru hago " + playername + " ezabatu nahi dukela ? ";
        reply = QMessageBox::question(this, "Bai", galdera, QMessageBox::Yes | QMessageBox::No );
        if (reply == QMessageBox::Yes)
        {
            // Si el usuario confirma, eliminar la fila
            removeRow(row);
        }
    }
}

void Taula::addPlayer(const Player &player)
{
    insertRow(rowCount());
    setItem(rowCount() - 1, PLAYER_ID_COLUMN, new QTableWidgetItem(QString("%1").arg(player.getId())));
    setItem(rowCount() - 1, PLAYER_NAME_COLUMN, new QTableWidgetItem(player.getName()));
}

QString Taula::getPlayerNameFromRow(int row)
{
    if (!item(row, PLAYER_NAME_COLUMN))
    {
        return "";
    }
    else
        return item(row, PLAYER_NAME_COLUMN)->text();
}

bool Taula::checkPlayerExists(const QString &name)
{
    for (auto row{0}; row < rowCount(); ++row)
    {
        if (item(row, PLAYER_NAME_COLUMN)->text() == name)
        {
            return true;
        }
    }
    return false;
}

int Taula::getRowTotalTimeInternal(int row)
{
    int total_time = 0;

    if (item(row, GAME1_TIME_COLUMN))
    {
        if (item(row, GAME1_TIME_COLUMN)->text() == "")
        {
            total_time += 10000;
        }
        total_time += item(row, GAME1_TIME_COLUMN)->text().toInt();
    }
    else
    {
        total_time += 10000;
    }

    if (item(row, GAME2_TIME_COLUMN))
    {
        if (item(row, GAME2_TIME_COLUMN)->text() == "")
        {
            total_time += 100000;
        }
        total_time += item(row, GAME2_TIME_COLUMN)->text().toInt();
    }
    else
    {
        total_time += 100000;
    }

    if (item(row, GAME3_TIME_COLUMN))
    {
        if (item(row, GAME3_TIME_COLUMN)->text() == "")
        {
            total_time += 1000000;
        }
        total_time += item(row, GAME3_TIME_COLUMN)->text().toInt();
    }
    else
    {
        total_time += 1000000;
    }

    return total_time;
}

int Taula::getRowTotalErrorsInternal(int row)
{
    int total_errors = 0;
    if (item(row, GAME1_ERRORS_COLUMN))
    {
        bool ok;
        item(row, GAME1_ERRORS_COLUMN)->text().toInt(&ok);
        if (!ok)
            total_errors += 1000;
        else
            total_errors += item(row, GAME1_ERRORS_COLUMN)->text().toInt();
    }
    else
    {
        total_errors += 10000;
    }

    if (item(row, GAME2_ERRORS_COLUMN))
    {
        bool ok;
        item(row, GAME2_ERRORS_COLUMN)->text().toInt(&ok);
        if (!ok)
            total_errors += 10000;
        else
            total_errors += item(row, GAME2_ERRORS_COLUMN)->text().toInt();
    }
    else
    {
        total_errors += 10000;
    }

    if (item(row, GAME3_ERRORS_COLUMN))
    {
        bool ok;
        item(row, GAME3_ERRORS_COLUMN)->text().toInt(&ok);
        if (!ok)
            total_errors += 100000;
        else
            total_errors += item(row, GAME3_ERRORS_COLUMN)->text().toInt();
    }
    else
    {
        total_errors += 100000;
    }

    return total_errors;
}

int Taula::getRowTotalErrorsReal(int row)
{
    int total_errors = 0;
    if (item(row, GAME1_ERRORS_COLUMN))
    {
        total_errors += item(row, GAME1_ERRORS_COLUMN)->text().toInt();
    }
    if (item(row, GAME2_ERRORS_COLUMN))
    {
        total_errors += item(row, GAME2_ERRORS_COLUMN)->text().toInt();
    }
    if (item(row, GAME3_ERRORS_COLUMN))
    {
        total_errors += item(row, GAME3_ERRORS_COLUMN)->text().toInt();
    }
    return total_errors;
}

int Taula::getRowTotalTimeReal(int row)
{
    int total_time = 0;
    if (item(row, GAME1_TIME_COLUMN))
    {
        total_time += item(row, GAME1_TIME_COLUMN)->text().toInt();
    }
    if (item(row, GAME2_TIME_COLUMN))
    {
        total_time += item(row, GAME2_TIME_COLUMN)->text().toInt();
    }
    if (item(row, GAME3_TIME_COLUMN))
    {
        total_time += item(row, GAME3_TIME_COLUMN)->text().toInt();
    }

    return total_time;
}