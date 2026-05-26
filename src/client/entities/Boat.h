//
// Created by Wiktor on 11.03.2026.
//

#ifndef BOAT_H
#define BOAT_H

#pragma once

#include <chrono>
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
    sf::Vector2f targetVelocity;

    float currentAngle;
    float targetAngle;
    float rotation;
    float throttle;

    float acceleration;
    float turnSpeed;

    float dragForward;
    float dragLateral;

    int points;
    float raceTime;
    bool finished;

public:
    explicit Boat(sf::Vector2f startPos);
    virtual ~Boat() = default;

    void updateLocal(float deltaTime);
    void updateRemote(float deltaTime);

    void setThrottle(float newThrottle);
    void setRotation(float rotation);
    void resetControls();
    void addExternalForce(sf::Vector2f force);

    bool isInCollider(sf::Vector2f position);

    void setPosition(sf::Vector2f newPosition);
    void setTargetPosition(sf::Vector2f newPosition);
    [[nodiscard]] sf::Vector2f getPosition() const;
    void setVelocity(sf::Vector2f newVelocity);
    void setCurrentAngle(float newRotation);
    void setTargetAngle(float newAngle);
    [[nodiscard]] float getCurrentAngle() const;
    [[nodiscard]] float getRotation() const;
    [[nodiscard]] sf::Vector2f getVelocity() const;
    void setTargetVelocity(sf::Vector2f newVelocity);
    [[nodiscard]] float getSpeed() const;
    [[nodiscard]] float getThrottle() const;

    void setPoints(int newPoints);
    void addPoints(int newPoints);
    [[nodiscard]] int getPoints() const;

    void setFinished(bool newFinished);
    bool isFinished() const;

    void setRaceTime(float newRaceTime);
    [[nodiscard]] float getRaceTime() const;
};

#endif //BOAT_H
