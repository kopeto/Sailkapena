#ifndef PLAYER_H
#define PLAYER_H

#include <QString>

#include <optional>
#include <vector>

struct Result
{
    std::optional<int> timeSeconds;
    std::optional<int> errors;
};

class Player
{
public:
    explicit Player(const QString &playerName);

    static int nextID();

    int id{0};
    QString name;
    std::vector<Result> results;
};

#endif // PLAYER_H
