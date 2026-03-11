//
// Created by Wiktor on 11.03.2026.
//

#include "Player.h"

constexpr float PI = 3.14159265f;

Player::Player(sf::Vector2f startPos) : Boat(startPos) {}

void Player::handleInput(float deltaTime) {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
        setThrottle(1.0f);
    } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
        setThrottle(-0.1f);
    } else {
        setThrottle(0.0f);
    }

    float steeringSpeed = 180.f;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
        targetAngle -= steeringSpeed * deltaTime;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
        targetAngle += steeringSpeed * deltaTime;
    }
}