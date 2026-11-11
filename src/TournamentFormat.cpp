#include "TournamentFormat.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSet>

namespace
{
QString optionalString(const QJsonObject &object, const QString &key, const QString &fallback)
{
    const QJsonValue value = object.value(key);
    return value.isString() && !value.toString().trimmed().isEmpty() ? value.toString().trimmed() : fallback;
}

bool readPositiveInteger(const QJsonObject &object, const QString &key, int &value, QString &error)
{
    const QJsonValue jsonValue = object.value(key);
    if (!jsonValue.isDouble() || jsonValue.toDouble() < 1 || jsonValue.toDouble() > 1440 ||
        jsonValue.toDouble() != static_cast<int>(jsonValue.toDouble()))
    {
        error = QStringLiteral("'%1' 1 eta 1440 arteko zenbaki oso bat izan behar da.").arg(key);
        return false;
    }
    value = jsonValue.toInt();
    return true;
}

QString criterionKey(ResultCriterion criterion)
{
    return criterion == ResultCriterion::Time ? QStringLiteral("time") : QStringLiteral("errors");
}
}

TournamentDefinition TournamentDefinition::defaultDefinition()
{
    TournamentDefinition definition;
    definition.formatId = QStringLiteral("sailkapena-default");
    definition.name = QStringLiteral("Txapelketa estandarra");

    RoundDefinition first;
    first.id = QStringLiteral("qualifier-1");
    first.name = QStringLiteral("Lehen txanda");
    first.durationMinutes = 5;
    first.criteria = {ResultCriterion::Errors, ResultCriterion::Time};
    first.advanceCount = std::nullopt;
    first.tiePolicy = TiePolicy::IncludeTies;

    RoundDefinition second;
    second.id = QStringLiteral("qualifier-2");
    second.name = QStringLiteral("Bigarren txanda");
    second.durationMinutes = 5;
    second.criteria = {ResultCriterion::Errors, ResultCriterion::Time};
    second.advanceCount = 6;
    second.tiePolicy = TiePolicy::IncludeTies;
    second.rankingScope = RankingScope::Cumulative;

    RoundDefinition final;
    final.id = QStringLiteral("final");
    final.name = QStringLiteral("Finala");
    final.durationMinutes = 5;
    final.criteria = {ResultCriterion::Errors, ResultCriterion::Time};

    definition.rounds = {first, second, final};
    return definition;
}

bool TournamentDefinition::fromJson(const QByteArray &json, TournamentDefinition &definition, QString &error)
{
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(json, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject())
    {
        error = QStringLiteral("JSON fitxategia ezin izan da irakurri: %1").arg(parseError.errorString());
        return false;
    }

    const QJsonObject root = document.object();
    const QJsonValue schemaVersion = root.value(QStringLiteral("schemaVersion"));
    if (!schemaVersion.isDouble() || schemaVersion.toDouble() != 1)
    {
        error = QStringLiteral("'schemaVersion' balioak 1 izan behar du.");
        return false;
    }

    TournamentDefinition parsed;
    parsed.formatId = optionalString(root, QStringLiteral("formatId"), QStringLiteral("custom-tournament"));
    parsed.name = optionalString(root, QStringLiteral("name"), QStringLiteral("Txapelketa"));

    const QJsonValue roundsValue = root.value(QStringLiteral("rounds"));
    if (!roundsValue.isArray() || roundsValue.toArray().isEmpty() || roundsValue.toArray().size() > 32)
    {
        error = QStringLiteral("'rounds' eremuak 1 eta 32 txanda artean izan behar ditu.");
        return false;
    }

    QSet<QString> roundIds;
    const QJsonArray rounds = roundsValue.toArray();
    for (qsizetype index = 0; index < rounds.size(); ++index)
    {
        if (!rounds.at(index).isObject())
        {
            error = QStringLiteral("%1. txandaren definizioa objektu bat izan behar da.").arg(index + 1);
            return false;
        }

        const QJsonObject roundObject = rounds.at(index).toObject();
        RoundDefinition round;
        round.id = optionalString(roundObject, QStringLiteral("id"), QStringLiteral("round-%1").arg(index + 1));
        round.name = optionalString(roundObject, QStringLiteral("name"), QStringLiteral("%1. txanda").arg(index + 1));
        if (!readPositiveInteger(roundObject, QStringLiteral("durationMinutes"), round.durationMinutes, error))
            return false;
        if (roundIds.contains(round.id))
        {
            error = QStringLiteral("Txanda bakoitzaren 'id' balioak bakarra izan behar du.");
            return false;
        }
        roundIds.insert(round.id);

        const QJsonValue criteriaValue = roundObject.value(QStringLiteral("criteria"));
        if (!criteriaValue.isArray() || criteriaValue.toArray().isEmpty())
        {
            error = QStringLiteral("'%1' txandak gutxienez irizpide bat izan behar du.").arg(round.name);
            return false;
        }
        QSet<QString> criteriaSeen;
        for (const QJsonValue &criterionValue : criteriaValue.toArray())
        {
            if (!criterionValue.isString())
            {
                error = QStringLiteral("'%1' txandako 'criteria' balioak testuak izan behar dira.").arg(round.name);
                return false;
            }
            const QString key = criterionValue.toString();
            if (key != QStringLiteral("time") && key != QStringLiteral("errors"))
            {
                error = QStringLiteral("'criteria' eremuak 'time' edo 'errors' bakarrik onartzen ditu.");
                return false;
            }
            if (criteriaSeen.contains(key))
            {
                error = QStringLiteral("Irizpide bat ezin da bi aldiz errepikatu txanda berean.");
                return false;
            }
            criteriaSeen.insert(key);
            round.criteria.append(key == QStringLiteral("time") ? ResultCriterion::Time : ResultCriterion::Errors);
        }

        const QString rankingScope = roundObject.value(QStringLiteral("rankingScope")).toString(QStringLiteral("round"));
        if (rankingScope == QStringLiteral("round"))
            round.rankingScope = RankingScope::Round;
        else if (rankingScope == QStringLiteral("cumulative"))
            round.rankingScope = RankingScope::Cumulative;
        else
        {
            error = QStringLiteral("'rankingScope' balioak 'round' edo 'cumulative' izan behar du.");
            return false;
        }

        const QString tiePolicy = roundObject.value(QStringLiteral("tiePolicy")).toString(QStringLiteral("includeTies"));
        if (tiePolicy == QStringLiteral("includeTies"))
            round.tiePolicy = TiePolicy::IncludeTies;
        else if (tiePolicy == QStringLiteral("manual"))
            round.tiePolicy = TiePolicy::Manual;
        else
        {
            error = QStringLiteral("'tiePolicy' balioak 'includeTies' edo 'manual' izan behar du.");
            return false;
        }

        const bool isFinalRound = index == rounds.size() - 1;
        const QJsonValue advanceValue = roundObject.value(QStringLiteral("advanceCount"));
        if (!isFinalRound)
        {
            if (advanceValue.isNull() || advanceValue.isUndefined())
                round.advanceCount = std::nullopt;
            else if (advanceValue.isDouble() && advanceValue.toDouble() >= 1 &&
                     advanceValue.toDouble() <= 1000000 && advanceValue.toDouble() == advanceValue.toInt())
                round.advanceCount = advanceValue.toInt();
            else
            {
                error = QStringLiteral("'%1' txandako 'advanceCount' zenbaki positiboa edo null izan behar da.").arg(round.name);
                return false;
            }
        }
        else if (!advanceValue.isUndefined() && !advanceValue.isNull())
        {
            error = QStringLiteral("Azken txandak ezin du 'advanceCount' izan.");
            return false;
        }

        parsed.rounds.append(round);
    }

    definition = std::move(parsed);
    error.clear();
    return true;
}

QByteArray TournamentDefinition::toJson() const
{
    QJsonObject root;
    root.insert(QStringLiteral("schemaVersion"), 1);
    root.insert(QStringLiteral("formatId"), formatId);
    root.insert(QStringLiteral("name"), name);

    QJsonArray roundsJson;
    for (qsizetype index = 0; index < rounds.size(); ++index)
    {
        const RoundDefinition &round = rounds.at(index);
        QJsonObject roundObject;
        roundObject.insert(QStringLiteral("id"), round.id);
        roundObject.insert(QStringLiteral("name"), round.name);
        roundObject.insert(QStringLiteral("durationMinutes"), round.durationMinutes);
        QJsonArray criteriaJson;
        for (ResultCriterion criterion : round.criteria)
            criteriaJson.append(criterionKey(criterion));
        roundObject.insert(QStringLiteral("criteria"), criteriaJson);
        roundObject.insert(QStringLiteral("rankingScope"), round.rankingScope == RankingScope::Cumulative
                                                                  ? QStringLiteral("cumulative") : QStringLiteral("round"));
        roundObject.insert(QStringLiteral("tiePolicy"), round.tiePolicy == TiePolicy::Manual
                                                              ? QStringLiteral("manual") : QStringLiteral("includeTies"));
        if (index < rounds.size() - 1)
            roundObject.insert(QStringLiteral("advanceCount"), round.advanceCount ? QJsonValue(*round.advanceCount) : QJsonValue(QJsonValue::Null));
        roundsJson.append(roundObject);
    }
    root.insert(QStringLiteral("rounds"), roundsJson);
    return QJsonDocument(root).toJson(QJsonDocument::Indented);
}

QString criterionName(ResultCriterion criterion)
{
    return criterion == ResultCriterion::Time ? QStringLiteral("Denbora") : QStringLiteral("Akatsak");
}
