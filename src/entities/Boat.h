//
// Created by Wiktor on 11.03.2026.
//

#ifndef BOAT_H
#define BOAT_H



#pragma once
#include <SFML/System/Vector2.hpp>

class Boat {
protected:
    sf::Vector2f position;
    sf::Vector2f velocity;

    float currentAngle;
    float targetAngle;

    float throttle;

    float acceleration;
    float turnSpeed;

    float dragForward;
    float dragLateral;

public:
    Boat(sf::Vector2f startPos);
    virtual ~Boat() = default;

    virtual void update(float deltaTime);

    void setThrottle(float newThrottle);
    void setTargetAngle(float angleInDegrees);

    sf::Vector2f getPosition() const;
    float getCurrentAngle() const;
    sf::Vector2f getVelocity() const;
    float getSpeed() const;
};



#endif //BOAT_H
