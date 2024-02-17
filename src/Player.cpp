#include "Player.h"


Player::Player(const QString& name) : name(name), id(Player::nextID()) 
{
    results.resize(3);
}

Player::Player(const char * name) : id(Player::nextID()) 
{
    this->name = QString(name);
    results.resize(3);
}

int Player::nextID()
{
    static int id = 1;
    return id++;
}

QString Player::getName() const
{
    return name;
}

int Player::getId() const
{
    return id;
}

void Player::setResult(int game, Emaitza_t result)
{
    results.at(game) = result;
}