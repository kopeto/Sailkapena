#include "Taula.h"
#include "Txapelketa.h"

#include <QMessageBox>
#include <QHeaderView>

static int time2Int(QString str)
{
    return str.mid(0, 2).toInt() * 3600 + str.mid(3, 2).toInt() * 60 + str.mid(6, 2).toInt();
}

static QString int2Time(int time)
{
    int hours = time / 3600;
    int minutes = (time / 60) % 60;
    int seconds = time % 60;

    return QString("%1:%2:%3").arg(hours, 2, 10, QChar('0')).arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));
}

static constexpr int FINALISTS_COUNT = 6;

Taula::Taula(QWidget *parent) : QTableWidget(parent)
{
    QStringList headers;

    headers << " "
            << " Jokalaria "
            << " 1. Proba "
            << "   "
            << " 2. Proba "
            << "   "
            << " Batera "
            << "   "
            << "   "
            << " Finala "
            << "   ";

    setColumnCount(headers.size());
    setColumnWidth(PLAYER_NAME_COLUMN, 250);
    setColumnWidth(GAME1_ERRORS_COLUMN, 75);
    setColumnWidth(GAME2_ERRORS_COLUMN, 75);
    setColumnWidth(GAME1_2_ERRORS_COLUMN, 75);
    setColumnWidth(FINALS_SEPARATOR_COLUMN, 2);
    horizontalHeader()->setSectionResizeMode(FINALS_SEPARATOR_COLUMN, QHeaderView::Fixed); // Fijar el tamaño
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
        reply = QMessageBox::question(this, "Bai", galdera, QMessageBox::Yes | QMessageBox::No);
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
        total_time += time2Int(item(row, GAME1_TIME_COLUMN)->text());
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

    return total_time;
}

void Taula::sort(GameState _state)
{
    tmpUpdate();

    sortByColumn(Taula::GAME1_2_TIME_COLUMN, Qt::SortOrder::AscendingOrder);
    sortByColumn(Taula::GAME1_2_ERRORS_COLUMN, Qt::SortOrder::AscendingOrder);

    if (_state == GameState::GAME_FINAL)
    {
        sortNRows(Taula::FINALS_TIME_COLUMN, FINALISTS_COUNT, Qt::SortOrder::AscendingOrder);
        sortNRows(Taula::FINALS_ERRORS_COLUMN, FINALISTS_COUNT, Qt::SortOrder::AscendingOrder);
    }

    updateValues();
}

// Sorts the first `rowsToSort` rows of the table using the given column.
// If both values can be converted to double, they will be compared numerically.
void Taula::sortNRows(int column, int rowsToSort, Qt::SortOrder order)
{
    const int totalRows = rowCount();
    const int totalCols = columnCount();
    if (totalRows <= 1 || totalCols == 0)
        return;

    rowsToSort = std::min(rowsToSort, totalRows);
    column = qBound(0, column, totalCols - 1);

    // Intermediate container: each row is a QVector of QTableWidgetItem*
    // that we own until setItem() takes ownership.
    struct RowData
    {
        QVector<QTableWidgetItem *> items; // length == totalCols
        int originalIndex;
    };

    QVector<RowData> rows;
    rows.reserve(rowsToSort);

    // 1) Copy (clone or create empty) the first rowsToSort rows
    for (int r = 0; r < rowsToSort; ++r)
    {
        RowData rd;
        rd.originalIndex = r;
        rd.items.resize(totalCols);
        for (int c = 1; c < totalCols; ++c)
        {
            QTableWidgetItem *it = item(r, c);
            if (it)
            {
                // clone() returns a new QTableWidgetItem* (owned by us)
                rd.items[c] = it->clone();
            }
            else
            {
                // create an empty item to keep consistency
                rd.items[c] = new QTableWidgetItem(QString());
            }
        }
        rows.append(std::move(rd));
    }

    // 2) Sort rows using a safe comparator (numeric when possible)
    auto cmp = [column, order](const RowData &a, const RowData &b) -> bool
    {
        const QString at = a.items[column]->text();
        const QString bt = b.items[column]->text();

        // If both values are numeric, compare as double
        bool aok = false, bok = false;
        const double ad = at.toDouble(&aok);
        const double bd = bt.toDouble(&bok);
        if (aok && bok)
        {
            if (order == Qt::AscendingOrder)
                return ad < bd;
            else
                return ad > bd;
        }
        // If one or both are non-numeric, fall back to textual compare

        // Text comparison (locale-insensitive). You can use locale-aware if needed.
        if (order == Qt::AscendingOrder)
            return at < bt;
        else
            return at > bt;
    };

    std::stable_sort(rows.begin(), rows.end(), cmp);

    // 3) Reinsert into the table: remove previous items and cellWidgets
    //    from these rows and insert the cloned sorted ones.
    for (int r = 0; r < rowsToSort; ++r)
    {
        for (int c = 1; c < totalCols; ++c)
        {
            // remove cell widget if present
            QWidget *w = cellWidget(r, c);
            if (w)
            {
                removeCellWidget(r, c);
                delete w;
            }

            // remove current item (takeItem returns the pointer, table releases ownership)
            QTableWidgetItem *old = takeItem(r, c);
            if (old)
                delete old;

            // set the new item (ownership is transferred to the table)
            setItem(r, c, rows[r].items[c]);
            // NOTE: we do not delete rows[r].items[c] because the table owns it now.
        }
    }

    // 4) Rows we transferred to the table should not be deleted.
    //    If anything was not transferred (should not happen), free it.
    for (int r = 0; r < rowsToSort; ++r)
    {
        for (int c = 1; c < totalCols; ++c)
        {
            // If the pointer was not transferred (defensive), set to null.
            // In this logic everything was transferred, so this does nothing.
            rows[r].items[c] = nullptr;
        }
    }
}

void Taula::tmpUpdate()
{
    for (auto row{0}; row < rowCount(); ++row)
    {
        int total_errors = getRowTotalErrorsInternal(row);
        int total_time = getRowTotalTimeInternal(row);

        QTableWidgetItem *totErrItem = new QTableWidgetItem();
        totErrItem->setData(Qt::EditRole, total_errors); // Accepts a QVariant
        setItem(row, Taula::GAME1_2_ERRORS_COLUMN, totErrItem);

        QTableWidgetItem *totTimeItem = new QTableWidgetItem();
        totTimeItem->setData(Qt::EditRole, total_time); // Accepts a QVariant
        setItem(row, Taula::GAME1_2_TIME_COLUMN, totTimeItem);
    }
}

void Taula::updateValues()
{
    for (auto row{0}; row < rowCount(); ++row)
    {
        int total_errors = getRowTotalErrorsReal(row);
        int total_time = getRowTotalTimeReal(row);

        QTableWidgetItem *totErrItem = new QTableWidgetItem(total_errors);
        totErrItem->setData(Qt::EditRole, total_errors);
        setItem(row, Taula::GAME1_2_ERRORS_COLUMN, totErrItem);

        QString total_time_show_data = int2Time(total_time);
        QTableWidgetItem *totTimeItem = new QTableWidgetItem(total_time_show_data);
        totTimeItem->setData(Qt::DisplayRole, total_time_show_data); // Accepts a QVariant
        totTimeItem->setData(Qt::UserRole, total_time);
        setItem(row, Taula::GAME1_2_TIME_COLUMN, totTimeItem);

        if (row >= FINALISTS_COUNT)
        {
            for (auto col{FINALS_SEPARATOR_COLUMN}; col < columnCount(); ++col)
            {
                // delete item
                QTableWidgetItem *it = takeItem(row, col);
                if (it)
                    delete it;
            }
        }
    }
}

void Taula::highlightFirstRows(int numRows)
{
    for (int row = 0; row < rowCount(); ++row)
    {
        for (int col = 0; col < columnCount(); ++col)
        {
            if (item(row, col) && col != FINALS_SEPARATOR_COLUMN)
            {
                if (row < numRows)
                {
                    item(row, col)->setBackground(QColor(210, 255, 210));
                }
                else
                {
                    item(row, col)->setBackground(QColor(255, 255, 255));
                }
            }
        }
    }
}