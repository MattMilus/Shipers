//
// Created by Wiktor on 11.03.2026.
//

#include "Boat.h"
#include <cmath>
#include <algorithm>

constexpr float PI = 3.14159265f;

Boat::Boat(sf::Vector2f startPos)
    : position(startPos), velocity(0.f, 0.f), currentAngle(0.f), targetAngle(0.f), throttle(0.f)
{
    acceleration = 300.f;
    turnSpeed = 120.f;

    dragForward = 0.98f;
    dragLateral = 0.85f;
}

void Boat::update(float deltaTime) {
    float angleDiff = targetAngle - currentAngle;

    while (angleDiff > 180.f) angleDiff -= 360.f;
    while (angleDiff < -180.f) angleDiff += 360.f;

    if (std::abs(angleDiff) > 0.1f) {
        float rotationStep = turnSpeed * deltaTime;
        if (std::abs(angleDiff) <= rotationStep) {
            currentAngle = targetAngle;
        } else {
            currentAngle += (angleDiff > 0.f ? rotationStep : -rotationStep);
        }
    }

    while (currentAngle >= 360.f) currentAngle -= 360.f;
    while (currentAngle < 0.f) currentAngle += 360.f;


    float rad = (currentAngle - 90.f) * (PI / 180.f);
    sf::Vector2f forwardVec(std::cos(rad), std::sin(rad));

    sf::Vector2f rightVec(-std::sin(rad), std::cos(rad));

    velocity.x += forwardVec.x * throttle * acceleration * deltaTime;
    velocity.y += forwardVec.y * throttle * acceleration * deltaTime;

    float forwardVelocity = (velocity.x * forwardVec.x) + (velocity.y * forwardVec.y);
    float lateralVelocity = (velocity.x * rightVec.x) + (velocity.y * rightVec.y);

    forwardVelocity *= std::pow(dragForward, deltaTime * 60.f);
    lateralVelocity *= std::pow(dragLateral, deltaTime * 60.f);

    velocity = (forwardVec * forwardVelocity) + (rightVec * lateralVelocity);

    position += velocity * deltaTime;
}

void Boat::setThrottle(float newThrottle) {
    throttle = std::clamp(newThrottle, -0.5f, 1.0f);
}

void Boat::setTargetAngle(float angleInDegrees) { targetAngle = angleInDegrees; }
sf::Vector2f Boat::getPosition() const { return position; }
float Boat::getCurrentAngle() const { return currentAngle; }
sf::Vector2f Boat::getVelocity() const { return velocity; }

float Boat::getSpeed() const {
    return std::sqrt((velocity.x * velocity.x) + (velocity.y * velocity.y));
}
