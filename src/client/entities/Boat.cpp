//
// Created by Wiktor on 11.03.2026.
//

#include "Boat.h"
#include <cmath>
#include <algorithm>
#include <chrono>
#include <SFML/Graphics/CircleShape.hpp>

constexpr float PI = 3.14159265f;

Boat::Boat(sf::Vector2f startPos)
    : position(startPos),
      targetPosition(startPos),
      velocity(0.f, 0.f),
      targetVelocity(0.f, 0.f),
      currentAngle(0.f),
      targetAngle(0.f),
      rotation(0.f),
      throttle(0.f)
{
    acceleration = 300.f;
    turnSpeed = 120.f;

    dragForward = 0.995f;
    dragLateral = 0.98f;

    points = 0;
    raceTime = -1.0f;
    finished = false;
}

void Boat::handleRotation(float deltaTime) {
    if (std::abs(rotation) > 0.01f) {
        currentAngle += rotation * turnSpeed * deltaTime;
    }

    while (currentAngle >= 360.f) currentAngle -= 360.f;
    while (currentAngle < 0.f) currentAngle += 360.f;
}

void Boat::move(float deltaTime) {
    float rad = (currentAngle - 90.f) * (PI / 180.f);
    sf::Vector2f forwardVec(std::cos(rad), std::sin(rad));
    sf::Vector2f rightVec(-std::sin(rad), std::cos(rad));

    float forwardVelocity = (velocity.x * forwardVec.x) + (velocity.y * forwardVec.y);
    float lateralVelocity = (velocity.x * rightVec.x) + (velocity.y * rightVec.y);

    forwardVelocity *= std::pow(dragForward, deltaTime * 60.f);
    lateralVelocity *= std::pow(dragLateral, deltaTime * 60.f);

    forwardVelocity += throttle * acceleration * deltaTime;

    velocity = (forwardVec * forwardVelocity) + (rightVec * lateralVelocity);

    position += velocity * deltaTime;
}

void Boat::updateLocal(float deltaTime) {
    this->handleRotation(deltaTime);
    this->move(deltaTime);
}

void Boat::updateRemote(float deltaTime) {
    // LERP linear interpolation
    // Value 10.0f is the speed of interpolation (the bigger, the faster,
    // but may be less smooth).
    float lerpFactor = std::min(1.0f, 10.0f * deltaTime);

    position.x += (targetPosition.x - position.x) * lerpFactor;
    position.y += (targetPosition.y - position.y) * lerpFactor;

    velocity.x += (targetVelocity.x - velocity.x) * lerpFactor;
    velocity.y += (targetVelocity.y - velocity.y) * lerpFactor;

    float angleDiff = targetAngle - currentAngle;
    while (angleDiff > 180.f) angleDiff -= 360.f;
    while (angleDiff < -180.f) angleDiff += 360.f;

    currentAngle += angleDiff * lerpFactor;

    while (currentAngle >= 360.f) currentAngle -= 360.f;
    while (currentAngle < 0.f) currentAngle += 360.f;
}

void Boat::setThrottle(float newThrottle) {
    throttle = std::clamp(newThrottle, -0.5f, 1.0f);
}

void Boat::setRotation(float newRotation) {
    rotation = newRotation;
}

void Boat::resetControls() {
    setThrottle(0.0f);
    setRotation(0.0f);
}

void Boat::addExternalForce(sf::Vector2f force) {
    velocity += force;
}

bool Boat::isInCollider(sf::Vector2f point) {
    float dx = position.x - point.x;
    float dy = position.y - point.y;
    return (dx * dx + dy * dy) <= (COLLIDER_RADIUS * COLLIDER_RADIUS);
}

void Boat::setPosition(sf::Vector2f newPosition) {
    position = newPosition;
}

void Boat::setTargetPosition(sf::Vector2f newPosition) {
    targetPosition = newPosition;
}

sf::Vector2f Boat::getPosition() const { return position; }

void Boat::setVelocity(sf::Vector2f newVelocity) {
    velocity = newVelocity;
}

void Boat::setCurrentAngle(float newRotation) {
    currentAngle = newRotation;
}

void Boat::setTargetAngle(float newAngle) {
    targetAngle = newAngle;
}

float Boat::getCurrentAngle() const { return currentAngle; }
float Boat::getRotation() const { return rotation; }
sf::Vector2f Boat::getVelocity() const { return velocity; }

void Boat::setTargetVelocity(sf::Vector2f newVelocity) {
    this->targetVelocity = newVelocity;
}

float Boat::getSpeed() const {
    return std::sqrt((velocity.x * velocity.x) + (velocity.y * velocity.y));
}

float Boat::getThrottle() const { return throttle; }

void Boat::setPoints(int newPoints) {
    points = newPoints;
}

void Boat::addPoints(int newPoints) {
    points += newPoints;
}

int Boat::getPoints() const {
    return points;
}

void Boat::setFinished(bool newFinished) {
    finished = newFinished;
}

bool Boat::isFinished() const {
    return finished;
}

void Boat::setRaceTime(float newRaceTime) {
    raceTime = newRaceTime;
}

float Boat::getRaceTime() const {
    return raceTime;
}
