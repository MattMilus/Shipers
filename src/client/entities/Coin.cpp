#include "Coin.h"

Coin::Coin()
    : position(0.f, 0.f), radius(8.f), active(true), index(-1) {
}

Coin::Coin(const sf::Vector2f& pos, float radius, int globalIndex)
    : position(pos), radius(radius), active(true), index(globalIndex) {
}

const sf::Vector2f& Coin::getPosition() const {
    return position;
}

float Coin::getRadius() const {
    return radius;
}

bool Coin::isActive() const {
    return active;
}

void Coin::setActive(bool a) {
    active = a;
}

int Coin::getIndex() const {
    return index;
}

void Coin::setPosition(const sf::Vector2f& pos) {
    position = pos;
}
