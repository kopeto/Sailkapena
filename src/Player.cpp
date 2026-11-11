#include "Player.h"

Player::Player(const QString &playerName)
    : id(nextID()), name(playerName)
{
}

int Player::nextID()
{
    static int idCounter = 1;
    return idCounter++;
}
