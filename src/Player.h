#ifndef PLAYER_H
#define PLAYER_H

#include <QString>

#include <array>
#include <optional>

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
    std::array<Result, 3> results{};
};

#endif // PLAYER_H
