#include "Player.h"


Player::Player(const QString& name) : name(name), id(Player::nextID()) {}

Player::Player(const char * name) : id(Player::nextID()) {
    this->name = QString(name);
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