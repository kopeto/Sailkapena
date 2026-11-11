#ifndef TIMEFORMAT_H
#define TIMEFORMAT_H

#include <QString>
#include <QStringList>
#include <QtGlobal>

#include <algorithm>
#include <limits>
#include <optional>

namespace TimeFormat
{
inline QString format(qint64 seconds)
{
    const qint64 hours = seconds / 3600;
    const qint64 minutes = (seconds / 60) % 60;
    const qint64 remainder = seconds % 60;
    return QString("%1:%2:%3")
        .arg(hours, 2, 10, QChar('0'))
        .arg(minutes, 2, 10, QChar('0'))
        .arg(remainder, 2, 10, QChar('0'));
}

inline std::optional<int> parse(const QString &text)
{
    const QStringList parts = text.split(':');
    if (parts.size() != 3 || parts[0].isEmpty() || parts[1].size() != 2 || parts[2].size() != 2)
        return std::nullopt;
    const auto digitsOnly = [](const QString &part) {
        return std::all_of(part.cbegin(), part.cend(), [](QChar character) {
            return character.unicode() >= '0' && character.unicode() <= '9';
        });
    };
    if (!digitsOnly(parts[0]) || !digitsOnly(parts[1]) || !digitsOnly(parts[2]))
        return std::nullopt;

    bool hoursOk = false;
    bool minutesOk = false;
    bool secondsOk = false;
    const qint64 hours = parts[0].toLongLong(&hoursOk);
    const int minutes = parts[1].toInt(&minutesOk);
    const int seconds = parts[2].toInt(&secondsOk);
    if (!hoursOk || !minutesOk || !secondsOk || hours < 0 ||
        hours > std::numeric_limits<int>::max() / 3600 || minutes < 0 || minutes > 59 || seconds < 0 || seconds > 59)
        return std::nullopt;

    const qint64 total = hours * 3600 + minutes * 60 + seconds;
    if (total > std::numeric_limits<int>::max())
        return std::nullopt;
    return static_cast<int>(total);
}
}

#endif // TIMEFORMAT_H
