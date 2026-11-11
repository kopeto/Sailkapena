#include "Txapelketa.h"

#include "CountDownWidget.h"
#include "Taula.h"
#include "TimeFormat.h"

#include <QAbstractButton>
#include <QFile>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QSaveFile>
#include <QSet>
#include <QStringConverter>
#include <QStringList>
#include <QTextStream>

#include <algorithm>
#include <limits>
#include <optional>

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

bool parseOptionalInteger(const QJsonValue &value, std::optional<int> &result)
{
    if (value.isNull() || value.isUndefined())
    {
        result.reset();
        return true;
    }
    if (!value.isDouble() || value.toDouble() < 0 || value.toDouble() > std::numeric_limits<int>::max() ||
        value.toDouble() != value.toInt())
        return false;
    result = value.toInt();
    return true;
}

bool parseLegacyTime(const QString &text, std::optional<int> &result)
{
    if (text.trimmed().isEmpty())
    {
        result.reset();
        return true;
    }
    result = TimeFormat::parse(text.trimmed());
    return result.has_value();
}

bool parseLegacyErrors(const QString &text, std::optional<int> &result)
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

QJsonObject resultToJson(const Result &result)
{
    QJsonObject object;
    object.insert(QStringLiteral("timeSeconds"), result.timeSeconds ? QJsonValue(*result.timeSeconds) : QJsonValue(QJsonValue::Null));
    object.insert(QStringLiteral("errors"), result.errors ? QJsonValue(*result.errors) : QJsonValue(QJsonValue::Null));
    return object;
}

std::optional<qint64> accumulatedResult(const Player &player,
                                       const QVector<RoundDefinition> &rounds,
                                       int throughRound,
                                       ResultCriterion criterion)
{
    qint64 total = 0;
    bool hasValue = false;
    for (int roundIndex = 0; roundIndex <= throughRound; ++roundIndex)
    {
        const RoundDefinition &round = rounds.at(roundIndex);
        const bool isScored = std::find(round.criteria.cbegin(), round.criteria.cend(), criterion) != round.criteria.cend();
        if (roundIndex >= static_cast<int>(player.results.size()))
        {
            if (isScored)
                return std::nullopt;
            continue;
        }
        const Result &result = player.results[static_cast<std::size_t>(roundIndex)];
        const std::optional<int> value = criterion == ResultCriterion::Time ? result.timeSeconds : result.errors;
        if (!value)
        {
            if (isScored)
                return std::nullopt;
            continue;
        }
        total += *value;
        hasValue = true;
    }
    return hasValue ? std::optional<qint64>(total) : std::nullopt;
}
}

bool Txapelketa::saveToFile()
{
    const QString filePath = QFileDialog::getSaveFileName(
        this, tr("Lehiaketa JSON fitxategi gisa gorde"), QString(), tr("JSON fitxategiak (*.json)"));
    if (filePath.isEmpty())
        return false;
    if (saveToFile_(filePath))
        return true;
    showMessage(this, QMessageBox::Warning, tr("Errorea"), tr("Ezin izan da lehiaketaren JSON fitxategia gorde."));
    return false;
}

bool Txapelketa::exportToCsv()
{
    const QString filePath = QFileDialog::getSaveFileName(
        this, tr("Emaitzak CSV fitxategi gisa esportatu"), QString(), tr("CSV fitxategiak (*.csv)"));
    if (filePath.isEmpty())
        return false;
    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        showMessage(this, QMessageBox::Warning, tr("Errorea"), tr("Ezin izan da CSV fitxategia esportatu."));
        return false;
    }

    QTextStream output(&file);
    output.setEncoding(QStringConverter::Utf8);
    QStringList headers{QStringLiteral("Jokalaria")};
    for (qsizetype roundIndex = 0; roundIndex < definition_.rounds.size(); ++roundIndex)
    {
        const RoundDefinition &round = definition_.rounds.at(roundIndex);
        for (ResultCriterion criterion : round.criteria)
            headers.append(QStringLiteral("%1: %2").arg(round.name, criterionName(criterion)));
        if (roundIndex + 1 < definition_.rounds.size() && round.advanceCount)
        {
            headers.append(tr("%1. txandako ebaketa: Denbora metatua").arg(roundIndex + 1));
            headers.append(tr("%1. txandako ebaketa: Akatsak metatuak").arg(roundIndex + 1));
        }
    }
    for (qsizetype index = 0; index < headers.size(); ++index)
    {
        if (index > 0)
            output << ';';
        output << escapeDelimitedField(headers.at(index));
    }
    output << '\n';

    std::vector<const Player *> orderedPlayers;
    for (int row = 0; row < table_->rowCount(); ++row)
    {
        const QTableWidgetItem *nameItem = table_->item(row, Taula::PLAYER_NAME_COLUMN);
        const Player *player = nameItem ? findPlayer(nameItem->data(Qt::UserRole).toInt()) : nullptr;
        if (player)
            orderedPlayers.push_back(player);
    }
    for (const Player &player : players_)
    {
        const bool alreadyIncluded = std::any_of(orderedPlayers.cbegin(), orderedPlayers.cend(), [&player](const Player *included) {
            return included->id == player.id;
        });
        if (!alreadyIncluded)
            orderedPlayers.push_back(&player);
    }

    for (const Player *playerPointer : orderedPlayers)
    {
        const Player &player = *playerPointer;
        QStringList values{player.name};
        for (qsizetype roundIndex = 0; roundIndex < definition_.rounds.size(); ++roundIndex)
        {
            const Result result = roundIndex < static_cast<qsizetype>(player.results.size())
                                      ? player.results[static_cast<std::size_t>(roundIndex)] : Result{};
            const RoundDefinition &round = definition_.rounds.at(roundIndex);
            for (ResultCriterion criterion : round.criteria)
            {
                if (criterion == ResultCriterion::Time)
                    values.append(result.timeSeconds ? TimeFormat::format(*result.timeSeconds) : QString());
                else
                    values.append(result.errors ? QString::number(*result.errors) : QString());
            }
            if (roundIndex + 1 < definition_.rounds.size() && round.advanceCount)
            {
                const std::optional<qint64> totalTime = accumulatedResult(player, definition_.rounds,
                                                                           static_cast<int>(roundIndex), ResultCriterion::Time);
                const std::optional<qint64> totalErrors = accumulatedResult(player, definition_.rounds,
                                                                             static_cast<int>(roundIndex), ResultCriterion::Errors);
                values.append(totalTime ? TimeFormat::format(*totalTime) : QString());
                values.append(totalErrors ? QString::number(*totalErrors) : QString());
            }
        }
        for (qsizetype index = 0; index < values.size(); ++index)
        {
            if (index > 0)
                output << ';';
            output << escapeDelimitedField(values.at(index));
        }
        output << '\n';
    }
    output.flush();
    if (output.status() != QTextStream::Ok || !file.commit())
    {
        showMessage(this, QMessageBox::Warning, tr("Errorea"), tr("Ezin izan da CSV fitxategia esportatu."));
        return false;
    }
    showMessage(this, QMessageBox::Information, tr("CSV esportatu"),
                tr("CSV fitxategia taula ikusteko da; saioa eta txapelketaren formatua berreskuratzeko, erabili JSON."));
    return true;
}

bool Txapelketa::loadFromFile()
{
    const QString filePath = QFileDialog::getOpenFileName(
        this, tr("Lehiaketa ireki"), QString(),
        tr("JSON saioak eta aurreko CSV saioak (*.json *.csv *.tmp);;Fitxategi guztiak (*)"));
    if (filePath.isEmpty())
        return false;
    return loadFromFile_(filePath);
}

bool Txapelketa::loadFromFile_(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
    {
        showMessage(this, QMessageBox::Warning, tr("Errorea"), tr("Ezin izan da fitxategia irakurri."));
        return false;
    }
    QByteArray data = file.readAll();
    if (data.startsWith("\xEF\xBB\xBF"))
        data.remove(0, 3);
    const QByteArray trimmedData = data.trimmed();

    if (trimmedData.startsWith('{'))
    {
        QJsonParseError parseError;
        const QJsonDocument document = QJsonDocument::fromJson(trimmedData, &parseError);
        if (parseError.error != QJsonParseError::NoError || !document.isObject())
        {
            showMessage(this, QMessageBox::Warning, tr("JSON baliogabea"),
                        tr("JSON fitxategia ezin izan da irakurri: %1").arg(parseError.errorString()));
            return false;
        }
        const QJsonObject root = document.object();
        const QJsonValue sessionVersion = root.value(QStringLiteral("schemaVersion"));
        if (root.value(QStringLiteral("kind")).toString() != QStringLiteral("tournamentSession") ||
            !sessionVersion.isDouble() || sessionVersion.toDouble() != 1)
        {
            showMessage(this, QMessageBox::Warning, tr("Fitxategi mota"),
                        tr("Fitxategi honek ez du lehiaketa-saio baten formatua. Formatua editatzeko leihoan kargatu."));
            return false;
        }

        TournamentDefinition loadedDefinition;
        QString formatError;
        const QJsonDocument definitionDocument(root.value(QStringLiteral("definition")).toObject());
        if (!TournamentDefinition::fromJson(definitionDocument.toJson(QJsonDocument::Compact), loadedDefinition, formatError))
        {
            showMessage(this, QMessageBox::Warning, tr("Formatua baliogabea"), formatError);
            return false;
        }

        const QJsonValue playersValue = root.value(QStringLiteral("players"));
        if (!playersValue.isArray())
        {
            showMessage(this, QMessageBox::Warning, tr("Saio baliogabea"), tr("'players' eremua falta da."));
            return false;
        }
        std::vector<Player> loadedPlayers;
        QString lineError;
        for (qsizetype playerIndex = 0; playerIndex < playersValue.toArray().size(); ++playerIndex)
        {
            const QJsonValue playerValue = playersValue.toArray().at(playerIndex);
            if (!playerValue.isObject())
            {
                lineError = tr("%1. jokalariaren datuak baliogabeak dira.").arg(playerIndex + 1);
                break;
            }
            const QJsonObject playerObject = playerValue.toObject();
            const QString name = playerObject.value(QStringLiteral("name")).toString().trimmed();
            if (name.isEmpty() || name.contains(QLatin1Char('\n')) || name.contains(QLatin1Char('\r')))
            {
                lineError = tr("%1. jokalariaren izena baliogabea da.").arg(playerIndex + 1);
                break;
            }
            const auto duplicate = std::find_if(loadedPlayers.cbegin(), loadedPlayers.cend(), [&name](const Player &player) {
                return player.name.compare(name, Qt::CaseInsensitive) == 0;
            });
            if (duplicate != loadedPlayers.cend())
            {
                lineError = tr("Jokalari izen bat errepikatuta dago: %1.").arg(name);
                break;
            }
            const QJsonValue resultsValue = playerObject.value(QStringLiteral("results"));
            if (!resultsValue.isArray() || resultsValue.toArray().size() != loadedDefinition.rounds.size())
            {
                lineError = tr("%1 jokalariak ez du txanda bakoitzeko emaitzarik.").arg(name);
                break;
            }
            Player player(name);
            player.results.resize(static_cast<std::size_t>(loadedDefinition.rounds.size()));
            for (qsizetype roundIndex = 0; roundIndex < resultsValue.toArray().size(); ++roundIndex)
            {
                if (!resultsValue.toArray().at(roundIndex).isObject())
                {
                    lineError = tr("%1 jokalariaren %2. txandako emaitzak baliogabeak dira.").arg(name).arg(roundIndex + 1);
                    break;
                }
                const QJsonObject resultObject = resultsValue.toArray().at(roundIndex).toObject();
                Result &result = player.results[static_cast<std::size_t>(roundIndex)];
                if (!parseOptionalInteger(resultObject.value(QStringLiteral("timeSeconds")), result.timeSeconds) ||
                    !parseOptionalInteger(resultObject.value(QStringLiteral("errors")), result.errors))
                {
                    lineError = tr("%1 jokalariaren %2. txandako emaitza zenbakizkoak baliogabeak dira.").arg(name).arg(roundIndex + 1);
                    break;
                }
            }
            if (!lineError.isEmpty())
                break;
            loadedPlayers.push_back(std::move(player));
        }
        if (!lineError.isEmpty())
        {
            showMessage(this, QMessageBox::Warning, tr("Saio baliogabea"), lineError);
            return false;
        }

        const QJsonValue activeRoundValue = root.value(QStringLiteral("activeRoundIndex"));
        if (!activeRoundValue.isDouble() || activeRoundValue.toDouble() != activeRoundValue.toInt() ||
            activeRoundValue.toInt(-1) < 0 || activeRoundValue.toInt() >= loadedDefinition.rounds.size())
        {
            showMessage(this, QMessageBox::Warning, tr("Saio baliogabea"), tr("Uneko txanda baliogabea da."));
            return false;
        }
        std::vector<std::vector<int>> loadedQualified(static_cast<std::size_t>(loadedDefinition.rounds.size()));
        const QJsonValue qualifiedValue = root.value(QStringLiteral("qualifiedPlayersByRound"));
        if (!qualifiedValue.isArray() || qualifiedValue.toArray().size() != loadedDefinition.rounds.size())
        {
            showMessage(this, QMessageBox::Warning, tr("Saio baliogabea"), tr("Txandaz pasatutako jokalarien zerrenda baliogabea da."));
            return false;
        }
        for (qsizetype roundIndex = 0; roundIndex < qualifiedValue.toArray().size(); ++roundIndex)
        {
            const QJsonValue namesValue = qualifiedValue.toArray().at(roundIndex);
            if (!namesValue.isArray())
            {
                showMessage(this, QMessageBox::Warning, tr("Saio baliogabea"), tr("Txandaz pasatutako jokalarien izenak baliogabeak dira."));
                return false;
            }
            QSet<QString> namesSeen;
            for (const QJsonValue &nameValue : namesValue.toArray())
            {
                if (!nameValue.isString() || namesSeen.contains(nameValue.toString().toCaseFolded()))
                {
                    showMessage(this, QMessageBox::Warning, tr("Saio baliogabea"), tr("Jokalari bat aurrerapen-zerrendan errepikatuta dago."));
                    return false;
                }
                namesSeen.insert(nameValue.toString().toCaseFolded());
                const auto player = std::find_if(loadedPlayers.cbegin(), loadedPlayers.cend(), [&nameValue](const Player &candidate) {
                    return candidate.name.compare(nameValue.toString(), Qt::CaseInsensitive) == 0;
                });
                if (player == loadedPlayers.cend())
                {
                    showMessage(this, QMessageBox::Warning, tr("Saio baliogabea"), tr("Aurrerapen-zerrendak existitzen ez den jokalaria dauka."));
                    return false;
                }
                loadedQualified[static_cast<std::size_t>(roundIndex)].push_back(player->id);
            }
        }

        const int loadedActiveRound = activeRoundValue.toInt();
        const bool loadedFinished = root.value(QStringLiteral("tournamentFinished")).toBool(false);
        for (int closedRound = 0; closedRound < loadedActiveRound; ++closedRound)
        {
            std::vector<int> participants;
            if (closedRound == 0)
            {
                for (const Player &player : loadedPlayers)
                    participants.push_back(player.id);
            }
            else
            {
                participants = loadedQualified[static_cast<std::size_t>(closedRound)];
            }
            if (participants.empty())
            {
                showMessage(this, QMessageBox::Warning, tr("Saio baliogabea"),
                            tr("Itxitako txanda batek ez du parte-hartzailerik."));
                return false;
            }
        }
        if (loadedActiveRound > 0 && loadedQualified[static_cast<std::size_t>(loadedActiveRound)].empty())
        {
            showMessage(this, QMessageBox::Warning, tr("Saio baliogabea"),
                        tr("Uneko txandak ez dauka sailkatutako jokalaririk."));
            return false;
        }
        if (loadedFinished)
        {
            if (loadedActiveRound != loadedDefinition.rounds.size() - 1)
            {
                showMessage(this, QMessageBox::Warning, tr("Saio baliogabea"),
                            tr("Amaitutako txapelketa batek azken txandan egon behar du."));
                return false;
            }
            const std::vector<int> finalParticipants = loadedActiveRound == 0
                ? [&loadedPlayers]() {
                    std::vector<int> ids;
                    for (const Player &player : loadedPlayers)
                        ids.push_back(player.id);
                    return ids;
                }()
                : loadedQualified[static_cast<std::size_t>(loadedActiveRound)];
            if (finalParticipants.empty())
            {
                showMessage(this, QMessageBox::Warning, tr("Saio baliogabea"),
                            tr("Amaitutako txapelketak azken txandan parte-hartzailerik ez dauka."));
                return false;
            }
        }

        definition_ = std::move(loadedDefinition);
        players_ = std::move(loadedPlayers);
        qualifiedIdsByRound_ = std::move(loadedQualified);
        activeRoundIndex_ = loadedActiveRound;
        tournamentFinished_ = loadedFinished;
        timer_->configureRound(definition_.rounds.at(activeRoundIndex_).durationMinutes,
                               definition_.rounds.at(activeRoundIndex_).name);
        updateRoundControls();
        refreshTable();
        saveCheckpoint();
        return true;
    }

    const QString csvText = QString::fromUtf8(data);
    const QStringList lines = csvText.split(QLatin1Char('\n'));
    std::vector<Player> loadedPlayers;
    QString lineError;
    int lineNumber = 0;
    for (const QString &line : lines)
    {
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
        const QString firstField = fields.value(0).trimmed();
        const QString secondField = fields.value(1).trimmed();
        if (firstField.compare(QStringLiteral("Jokalaria"), Qt::CaseInsensitive) == 0 &&
            (secondField.compare(QStringLiteral("T1"), Qt::CaseInsensitive) == 0 || secondField.contains(QStringLiteral("proba"), Qt::CaseInsensitive)))
            continue;
        if (fields.size() != 9 && fields.size() != 10)
        {
            lineError = tr("%1. lerroak 9 edo 10 zutabe izan behar ditu; CSV formatua esportaziorako baino ez da.").arg(lineNumber);
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
                lineError = tr("%1. lerroak hiru probako emaitzak dauzka; ezin da aurreko formatuarekin galdu gabe kargatu.").arg(lineNumber);
                break;
            }
        }
        const QString name = fields[0].trimmed();
        if (name.isEmpty() || name.contains(QLatin1Char('\n')) || name.contains(QLatin1Char('\r')))
        {
            lineError = tr("%1. lerroan jokalariaren izena falta da edo baliogabea da.").arg(lineNumber);
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
        player.results.resize(3);
        const int finalTimeIndex = fields.size() == 10 ? 8 : 7;
        const int finalErrorsIndex = fields.size() == 10 ? 9 : 8;
        if (!parseLegacyTime(fields[1], player.results[0].timeSeconds) ||
            !parseLegacyErrors(fields[2], player.results[0].errors) ||
            !parseLegacyTime(fields[3], player.results[1].timeSeconds) ||
            !parseLegacyErrors(fields[4], player.results[1].errors) ||
            !parseLegacyTime(fields[finalTimeIndex], player.results[2].timeSeconds) ||
            !parseLegacyErrors(fields[finalErrorsIndex], player.results[2].errors))
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

    definition_ = TournamentDefinition::defaultDefinition();
    players_ = std::move(loadedPlayers);
    for (Player &player : players_)
        player.results.resize(static_cast<std::size_t>(definition_.rounds.size()));
    qualifiedIdsByRound_.assign(static_cast<std::size_t>(definition_.rounds.size()), {});
    for (const Player &player : players_)
        qualifiedIdsByRound_[1].push_back(player.id);
    const std::vector<Player> preliminaryRanking = rankedEligiblePlayers(1);
    const int finalistCount = std::min<int>(definition_.rounds.at(1).advanceCount.value_or(static_cast<int>(preliminaryRanking.size())),
                                            static_cast<int>(preliminaryRanking.size()));
    for (int index = 0; index < finalistCount; ++index)
        qualifiedIdsByRound_[2].push_back(preliminaryRanking[static_cast<std::size_t>(index)].id);

    bool hasSecondRoundResult = false;
    bool hasFinalResult = false;
    for (const Player &player : players_)
    {
        hasSecondRoundResult = hasSecondRoundResult || player.results[1].timeSeconds || player.results[1].errors;
        hasFinalResult = hasFinalResult || player.results[2].timeSeconds || player.results[2].errors;
    }
    activeRoundIndex_ = hasFinalResult ? 2 : hasSecondRoundResult ? 1 : 0;
    tournamentFinished_ = false;
    timer_->configureRound(definition_.rounds.at(activeRoundIndex_).durationMinutes,
                           definition_.rounds.at(activeRoundIndex_).name);
    updateRoundControls();
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

    QJsonObject session;
    session.insert(QStringLiteral("kind"), QStringLiteral("tournamentSession"));
    session.insert(QStringLiteral("schemaVersion"), 1);
    session.insert(QStringLiteral("definition"), QJsonDocument::fromJson(definition_.toJson()).object());
    session.insert(QStringLiteral("activeRoundIndex"), activeRoundIndex_);
    session.insert(QStringLiteral("tournamentFinished"), tournamentFinished_);

    QJsonArray playersJson;
    for (const Player &player : players_)
    {
        QJsonObject playerObject;
        playerObject.insert(QStringLiteral("name"), player.name);
        QJsonArray resultsJson;
        for (qsizetype roundIndex = 0; roundIndex < definition_.rounds.size(); ++roundIndex)
        {
            const Result result = roundIndex < static_cast<qsizetype>(player.results.size())
                                      ? player.results[static_cast<std::size_t>(roundIndex)] : Result{};
            resultsJson.append(resultToJson(result));
        }
        playerObject.insert(QStringLiteral("results"), resultsJson);
        playersJson.append(playerObject);
    }
    session.insert(QStringLiteral("players"), playersJson);

    QJsonArray qualifiedJson;
    for (const std::vector<int> &qualifiedIds : qualifiedIdsByRound_)
    {
        QJsonArray namesJson;
        for (int playerId : qualifiedIds)
        {
            const Player *player = findPlayer(playerId);
            if (player)
                namesJson.append(player->name);
        }
        qualifiedJson.append(namesJson);
    }
    session.insert(QStringLiteral("qualifiedPlayersByRound"), qualifiedJson);

    const QByteArray json = QJsonDocument(session).toJson(QJsonDocument::Indented);
    if (file.write(json) != json.size())
    {
        file.cancelWriting();
        return false;
    }
    return file.commit();
}
