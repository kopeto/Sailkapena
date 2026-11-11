#ifndef TOURNAMENTFORMAT_H
#define TOURNAMENTFORMAT_H

#include <QByteArray>
#include <QString>
#include <QVector>

#include <optional>

enum class ResultCriterion
{
    Time,
    Errors
};

enum class TiePolicy
{
    IncludeTies,
    Manual
};

enum class RankingScope
{
    Round,
    Cumulative
};

struct RoundDefinition
{
    QString id;
    QString name;
    int durationMinutes{5};
    QVector<ResultCriterion> criteria;
    std::optional<int> advanceCount;
    TiePolicy tiePolicy{TiePolicy::IncludeTies};
    RankingScope rankingScope{RankingScope::Round};
};

struct TournamentDefinition
{
    QString formatId;
    QString name;
    QVector<RoundDefinition> rounds;

    static TournamentDefinition defaultDefinition();
    static bool fromJson(const QByteArray &json, TournamentDefinition &definition, QString &error);
    QByteArray toJson() const;
};

QString criterionName(ResultCriterion criterion);

#endif // TOURNAMENTFORMAT_H
