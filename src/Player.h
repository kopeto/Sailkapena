#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include <QPointer>
#include <QString>

#include <vector>
#include <utility>

using Akatsak_t = int;
using Denbora_t = int;
using Emaitza_t = std::pair<Denbora_t, Akatsak_t>;

class Player
{
public:
    Player() = default;
    Player(const Player& other) = default;
    Player(Player&& other) = default;
    Player& operator=(const Player& other) = default;
    Player& operator=(Player&& other) = default;

    Player(const QString& name);
    Player(const char *name);

    /**
     * Destructor
    */
    ~Player() = default;
    
    /**
     * Initialize player ids
    */
    static int nextID();

    /**
     * Add a result
    */
    void addResult(Emaitza_t result);

    /**
     * Get player name
    */
    QString getName() const;

    /**
     * Get player id
    */
    int getId() const;

    QString name;
    int id;
    std::vector<Emaitza_t> results{};
};

#endif // PLAYER_H