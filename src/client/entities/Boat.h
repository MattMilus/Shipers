//
// Created by Wiktor on 11.03.2026.
//

#ifndef BOAT_H
#define BOAT_H



#pragma once
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>
#include "../env.h"

#define COLLIDER_RADIUS 20.f

class Boat {
private:
    void handleRotation(float deltaTime);
    void move(float deltaTime);
protected:
    sf::Vector2f position;
    sf::Vector2f targetPosition;
    sf::Vector2f velocity;

    float currentAngle;
    float targetAngle;
    float angleCommand;

    float throttle;

    float acceleration;
    float turnSpeed;

    float dragForward;
    float dragLateral;

public:
    Boat(sf::Vector2f startPos);
    virtual ~Boat() = default;

    void updateLocal(float deltaTime);
    void updateRemote(float deltaTime);

    void setThrottle(float newThrottle);
    void addToAngleCommand(float angleInDegrees);
    void addExternalForce(sf::Vector2f force);

    bool isInCollider(sf::Vector2f position);

    void setPosition(sf::Vector2f newPosition);
    void setTargetPosition(sf::Vector2f newPosition);
    sf::Vector2f getPosition() const;
    void setCurrentAngle(float newRotation );
    void setTargetAngle(float newAngle);
    float getCurrentAngle() const;
    float getAngleCommand() const;
    sf::Vector2f getVelocity() const;
    float getSpeed() const;
    float getThrottle() const;

    void debug(sf::RenderTarget& target);
};



#endif //BOAT_H
