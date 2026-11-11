#include "Txapelketa.h"

#include "TimeFormat.h"

#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QSaveFile>
#include <QStringConverter>
#include <QStringList>
#include <QTextStream>

#include <algorithm>
#include <optional>
#include <utility>

namespace
{
void showMessage(QWidget *parent, QMessageBox::Icon icon, const QString &title, const QString &text)
{
    QMessageBox message(icon, title, text, QMessageBox::NoButton, parent);
    message.addButton(QObject::tr("Ados"), QMessageBox::AcceptRole);
    message.exec();
}

bool parseDelimitedLine(const QString &line, QStringList &fields)
{
    QString field;
    bool insideQuotes = false;
    bool afterQuote = false;

    for (qsizetype index = 0; index < line.size(); ++index)
    {
        const QChar character = line.at(index);
        if (insideQuotes)
        {
            if (character == QLatin1Char('"'))
            {
                if (index + 1 < line.size() && line.at(index + 1) == QLatin1Char('"'))
                {
                    field += QLatin1Char('"');
                    ++index;
                }
                else
                {
                    insideQuotes = false;
                    afterQuote = true;
                }
            }
            else
            {
                field += character;
            }
            continue;
        }

        if (afterQuote)
        {
            if (character == QLatin1Char(';'))
            {
                fields.append(field);
                field.clear();
                afterQuote = false;
            }
            else if (!character.isSpace())
            {
                return false;
            }
            continue;
        }

        if (character == QLatin1Char(';'))
        {
            fields.append(field);
            field.clear();
        }
        else if (character == QLatin1Char('"'))
        {
            if (!field.isEmpty())
                return false;
            insideQuotes = true;
        }
        else
        {
            field += character;
        }
    }

    if (insideQuotes)
        return false;
    fields.append(field);
    return true;
}

QString escapeDelimitedField(const QString &value)
{
    if (!value.contains(QLatin1Char(';')) && !value.contains(QLatin1Char('"')) &&
        !value.contains(QLatin1Char('\n')) && !value.contains(QLatin1Char('\r')))
        return value;

    QString escaped = value;
    escaped.replace("\"", "\"\"");
    return QLatin1Char('"') + escaped + QLatin1Char('"');
}

bool parseErrors(const QString &text, std::optional<int> &result)
{
    if (text.trimmed().isEmpty())
    {
        result.reset();
        return true;
    }

    bool ok = false;
    const int errors = text.trimmed().toInt(&ok);
    if (!ok || errors < 0)
        return false;
    result = errors;
    return true;
}

bool parseTime(const QString &text, std::optional<int> &result)
{
    if (text.trimmed().isEmpty())
    {
        result.reset();
        return true;
    }

    result = TimeFormat::parse(text.trimmed());
    return result.has_value();
}
}

bool Txapelketa::saveToFile()
{
    const QString filePath = QFileDialog::getSaveFileName(
        this, tr("Emaitzak gorde"), QString(), tr("CSV fitxategiak (*.csv);;Testu fitxategiak (*.txt)"));
    if (filePath.isEmpty())
        return false;

    if (saveToFile_(filePath))
        return true;
    showMessage(this, QMessageBox::Warning, tr("Errorea"), tr("Ezin izan da fitxategia gorde."));
    return false;
}

bool Txapelketa::exportToCsv()
{
    const QString filePath = QFileDialog::getSaveFileName(
        this, tr("Emaitzak CSV fitxategi gisa esportatu"), QString(), tr("CSV fitxategiak (*.csv)"));
    if (filePath.isEmpty())
        return false;

    if (saveToFile_(filePath))
        return true;
    showMessage(this, QMessageBox::Warning, tr("Errorea"), tr("Ezin izan da CSV fitxategia esportatu."));
    return false;
}

bool Txapelketa::loadFromFile()
{
    const QString filePath = QFileDialog::getOpenFileName(
        this, tr("Emaitzak ireki"), QString(),
        tr("CSV fitxategiak (*.csv *.tmp);;Testu fitxategiak (*.txt);;Fitxategi guztiak (*)"));
    if (filePath.isEmpty())
        return false;
    return loadFromFile_(filePath);
}

bool Txapelketa::loadFromFile_(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        showMessage(this, QMessageBox::Warning, tr("Errorea"), tr("Ezin izan da fitxategia irakurri."));
        return false;
    }

    QTextStream input(&file);
    input.setEncoding(QStringConverter::Utf8);
    std::vector<Player> loadedPlayers;
    QString lineError;
    int lineNumber = 0;

    while (!input.atEnd())
    {
        const QString line = input.readLine();
        ++lineNumber;
        const QString parsedLine = lineNumber == 1 ? QString(line).remove(QChar(0xFEFF)) : line;
        if (parsedLine.trimmed().isEmpty())
            continue;

        QStringList fields;
        if (!parseDelimitedLine(parsedLine, fields))
        {
            lineError = tr("%1. lerroak CSV formatua urratzen du.").arg(lineNumber);
            break;
        }
        const QString firstField = fields.front().trimmed();
        const QString secondField = fields.value(1).trimmed();
        if (firstField.compare("Jokalaria", Qt::CaseInsensitive) == 0 &&
            (secondField.compare("T1", Qt::CaseInsensitive) == 0 ||
             secondField.contains("proba", Qt::CaseInsensitive)))
            continue;
        if (fields.size() != 9 && fields.size() != 10)
        {
            lineError = tr("%1. lerroak 9 edo 10 zutabe izan behar ditu.").arg(lineNumber);
            break;
        }
        if (fields.size() == 10 && !fields[7].trimmed().isEmpty())
        {
            lineError = tr("%1. lerroko bereizketa-zutabeak hutsik egon behar du.").arg(lineNumber);
            break;
        }

        if (fields.size() == 9)
        {
            const auto firstTime = TimeFormat::parse(fields[1].trimmed());
            const auto secondTime = TimeFormat::parse(fields[3].trimmed());
            const auto thirdTime = TimeFormat::parse(fields[5].trimmed());
            const auto combinedTime = TimeFormat::parse(fields[7].trimmed());
            if (firstTime && secondTime && thirdTime && combinedTime &&
                static_cast<qint64>(*firstTime) + *secondTime + *thirdTime == *combinedTime)
            {
                lineError = tr("%1. lerroak hiru probaren emaitzak dauzka; formatu hori ezin da galdu gabe kargatu.").arg(lineNumber);
                break;
            }
        }

        const QString name = fields[0].trimmed();
        if (name.isEmpty())
        {
            lineError = tr("%1. lerroan jokalariaren izena falta da.").arg(lineNumber);
            break;
        }
        if (name.contains(QLatin1Char('\n')) || name.contains(QLatin1Char('\r')))
        {
            lineError = tr("%1. lerroko jokalariaren izenak ezin du lerro-jauzirik izan.").arg(lineNumber);
            break;
        }
        const auto duplicate = std::find_if(loadedPlayers.cbegin(), loadedPlayers.cend(), [&name](const Player &player) {
            return player.name.compare(name, Qt::CaseInsensitive) == 0;
        });
        if (duplicate != loadedPlayers.cend())
        {
            lineError = tr("%1. lerroan jokalaria errepikatuta dago.").arg(lineNumber);
            break;
        }

        Player player(name);
        const int finalTimeIndex = fields.size() == 10 ? 8 : 7;
        const int finalErrorsIndex = fields.size() == 10 ? 9 : 8;
        if (!parseTime(fields[1], player.results[0].timeSeconds) ||
            !parseErrors(fields[2], player.results[0].errors) ||
            !parseTime(fields[3], player.results[1].timeSeconds) ||
            !parseErrors(fields[4], player.results[1].errors) ||
            !parseTime(fields[finalTimeIndex], player.results[2].timeSeconds) ||
            !parseErrors(fields[finalErrorsIndex], player.results[2].errors))
        {
            lineError = tr("%1. lerroak denbora edo akats kopuru baliogabea dauka.").arg(lineNumber);
            break;
        }
        loadedPlayers.push_back(std::move(player));
    }

    if (!lineError.isEmpty())
    {
        showMessage(this, QMessageBox::Warning, tr("Errorea"), lineError);
        return false;
    }

    file.close();
    players_ = std::move(loadedPlayers);
    refreshTable();
    saveCheckpoint();
    return true;
}

bool Txapelketa::saveToFile_(const QString &filePath) const
{
    if (filePath.isEmpty())
        return false;

    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream output(&file);
    output.setEncoding(QStringConverter::Utf8);
    const QStringList headers{
        "Jokalaria", "1. proba", "1. probako akatsak", "2. proba", "2. probako akatsak",
        "Guztira", "Guztizko akatsak", "", "Finala", "Finaleko akatsak"
    };
    for (int index = 0; index < headers.size(); ++index)
    {
        if (index > 0)
            output << ';';
        output << escapeDelimitedField(headers[index]);
    }
    output << '\n';

    const std::vector<Player> orderedPlayers = currentDisplayOrder();
    for (const Player &player : orderedPlayers)
    {
        qint64 totalTime = 0;
        qint64 totalErrors = 0;
        bool hasPreliminaryTime = false;
        for (int game = 0; game < 2; ++game)
        {
            const Result &result = player.results[static_cast<std::size_t>(game)];
            if (result.timeSeconds)
            {
                totalTime += *result.timeSeconds;
                totalErrors += result.errors.value_or(0);
                hasPreliminaryTime = true;
            }
        }

        const Result &first = player.results[0];
        const Result &second = player.results[1];
        const Result &final = player.results[2];
        const QStringList values{
            player.name,
            first.timeSeconds ? TimeFormat::format(*first.timeSeconds) : QString(),
            first.errors ? QString::number(*first.errors) : QString(),
            second.timeSeconds ? TimeFormat::format(*second.timeSeconds) : QString(),
            second.errors ? QString::number(*second.errors) : QString(),
            hasPreliminaryTime ? TimeFormat::format(totalTime) : QString(),
            hasPreliminaryTime ? QString::number(totalErrors) : QString(),
            QString(),
            final.timeSeconds ? TimeFormat::format(*final.timeSeconds) : QString(),
            final.errors ? QString::number(*final.errors) : QString()
        };

        for (int index = 0; index < values.size(); ++index)
        {
            if (index > 0)
                output << ';';
            output << escapeDelimitedField(values[index]);
        }
        output << '\n';
    }

    output.flush();
    if (output.status() != QTextStream::Ok)
    {
        file.cancelWriting();
        return false;
    }
    return file.commit();
}
