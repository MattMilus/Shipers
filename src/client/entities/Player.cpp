//
// Created by Wiktor on 11.03.2026.
//

#include "Player.h"

constexpr float PI = 3.14159265f;

Player::Player(int playerId, sf::Vector2f startPos) : Boat(startPos), playerId(playerId) {}

void Player::handleInput(float deltaTime) {

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
        setThrottle(1.0f);
    } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
        setThrottle(-0.2f);
    } else {
        setThrottle(0.0f);
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
       setRotation(-1.f);
    } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
        setRotation(1.f);
    } else {
        setRotation(0.0f);
    }
}

int Player::getPlayerId() const {
    return playerId;
}