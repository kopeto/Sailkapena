#include "Taula.h"
#include "Txapelketa.h"

#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>

bool Txapelketa::loadFromFile()
{
    QString filePath = QFileDialog::getOpenFileName(nullptr, "Aukeratu fitxategia", "", "Fitxategi guztiak (*);;CSV fitxategiak (*.csv);;Testu fitxategiak (*.txt)");
    if (filePath.isEmpty())
    {
        return false;
    }
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, "Error", "Ezin fitxategia irakurri.");
        return false;
    }

    // Reset Players
    Players.clear();

    QTextStream in(&file);
    mainLayout->removeWidget(taula);
    taula = new Taula(this);
    mainLayout->addWidget(taula);

    int row = 0;
    while (!in.atEnd())
    {
        QString line = in.readLine();
        QStringList parts = line.split(",");

        addPlayerNamed(parts[0]);

        int col = 2;
        for (const auto& part: parts)
        {
            QTableWidgetItem *item = new QTableWidgetItem(part);
            taula->setItem(row, col, item);
            col++;
        }
        ++row;
    }

    file.close();
    return true;
}

bool Txapelketa::saveToFile()
{
    QString filePath = QFileDialog::getSaveFileName(nullptr, "Aukeratu fitxategia", "", "Fitxategi guztiak (*);;CSV fitxategiak (*.csv);;Testu fitxategiak (*.txt)");
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, "Error", "Ezin fitxategia ireki.");
        return false;
    }

    QTextStream out(&file);

    // Escribir datos en formato CSV
    for (int row = 0; row < taula->rowCount(); ++row)
    {
        for (int col = 2; col < taula->columnCount()-2; ++col)
        {
            if (col > 2)
            {
                out << ",";
            }
            QTableWidgetItem *item = taula->item(row, col);
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

