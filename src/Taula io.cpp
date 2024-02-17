#include "Taula.h"
#include "Txapelketa.h"

#include <QMessageBox>
#include <QFile>
#include <QTextStream>

bool Taula::saveToFile(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, "Error", "Ezin fitxategia ireki.");
        return false;
    }

    QTextStream out(&file);

    // Escribir datos en formato CSV
    for (int row = 0; row < rowCount(); ++row)
    {
        for (int col = 0; col < columnCount(); ++col)
        {
            if (col > 0)
            {
                out << ",";
            }
            QTableWidgetItem *item = this->item(row, col);
            if (item)
            {
                out << item->text();
            }
        }
        out << "\n";
    }

    file.close();
    return true;
}

bool Taula::loadFromFile(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, "Error", "Ezin fitxategia irakurri.");
        return false;
    }

    QTextStream in(&file);

    clear();

    int row = 0;
    while (!in.atEnd())
    {
        QString line = in.readLine();
        QStringList parts = line.split(",");
        setRowCount(row + 1);
        for (int col = 0; col < parts.size(); ++col)
        {
            QTableWidgetItem *item = new QTableWidgetItem(parts[col]);
            setItem(row, col, item);
        }
        ++row;
    }

    file.close();
    return true;
}