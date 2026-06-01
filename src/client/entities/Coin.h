// Created by Copilot on demand
#ifndef COIN_H
#define COIN_H

#include <SFML/System/Vector2.hpp>

class Coin {
public:
    Coin();
    Coin(const sf::Vector2f& pos, float radius, int globalIndex);

    [[nodiscard]] const sf::Vector2f& getPosition() const;
    [[nodiscard]] float getRadius() const;
    [[nodiscard]] bool isActive() const;
    void setActive(bool a);
    [[nodiscard]] int getIndex() const;

    void setPosition(const sf::Vector2f& pos);

private:
    sf::Vector2f position;
    float radius;
    bool active;
    int index; // global index 0..63
};

#endif // COIN_H
