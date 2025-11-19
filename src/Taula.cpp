#include "Taula.h"
#include "Txapelketa.h"

#include <QMessageBox>

static int time2Int(QString str)
{
    return str.mid(0, 2).toInt() * 3600 + str.mid(3, 2).toInt() * 60 + str.mid(6, 2).toInt();
}

Taula::Taula(QWidget *parent) : QTableWidget(parent)
{
    QStringList headers;

    headers << " "
            << " Jokalaria "
            << " T1 "
            << "   "
            << " T2 "
            << "   "
            << " T3 "
            << "   "
            << " Denbora "
            << " Akatsak ";
            
    setColumnCount(headers.size());
    setColumnWidth(PLAYER_NAME_COLUMN, 250);
    setColumnWidth(GAME1_ERRORS_COLUMN, 75);
    setColumnWidth(GAME2_ERRORS_COLUMN, 75);
    setColumnWidth(GAME3_ERRORS_COLUMN, 75);
    setHorizontalHeaderLabels(headers);
    setMinimumWidth(1280);


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
        QString galdera = playername + " ezabatu? ";
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
    // setItem(rowCount() - 1, PLAYER_ID_COLUMN, new QTableWidgetItem(QString("%1").arg(player.getId())));
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
        total_time += time2Int (item(row, GAME1_TIME_COLUMN)->text());
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
        total_time += time2Int(item(row, GAME2_TIME_COLUMN)->text());
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
        total_time += time2Int( item(row, GAME3_TIME_COLUMN)->text());
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
        total_time += time2Int(item(row, GAME1_TIME_COLUMN)->text());
    }
    if (item(row, GAME2_TIME_COLUMN))
    {
        total_time += time2Int(item(row, GAME2_TIME_COLUMN)->text());
    }
    if (item(row, GAME3_TIME_COLUMN))
    {
        total_time += time2Int(item(row, GAME3_TIME_COLUMN)->text());
    }

    return total_time;
}