//
// Created by Wiktor on 12.03.2026.
//

#include "GameManager.h"
#include <cmath>

#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/Socket.hpp"
#include "web_managers/ConnectionManager.h"

GameManager::GameManager() : playerId(0) {
    // Initialize any necessary game state here
}

int GameManager::connectToServer() {
    const int id = ConnectionManager::connectToServer();
    if (id == -1) {
        return -1;
    }

    playerId = id;
    addPlayer(id, sf::Vector2f(400.f, 300.f));
    return playerId;
}

int GameManager::addPlayer(int id, sf::Vector2f startPos) {
    {
        if (activeBoats.size() >= 4) {
            return -1;
        }
        activeBoats[id] = std::make_unique<Player>(startPos);
        return id;
    }
}

int GameManager::addBoat(int id, sf::Vector2f startPos) {
    {
        if (activeBoats.size() >= 4) {
            return -1;
        }
        activeBoats[id] = std::make_unique<Boat>(startPos);
        return id;
    }
}

const std::map<int, std::unique_ptr<Boat>>& GameManager::getActiveBoats() const {
    return activeBoats;
}

Boat* GameManager::getBoatById(const int id) const {
    auto it = activeBoats.find(id);
    if (it != activeBoats.end()) {
        return it->second.get();
    }
    return nullptr;
}

Player* GameManager::getPlayer() const {
    auto it = activeBoats.find(playerId);
    if (it != activeBoats.end()) {
        Player* playerPtr = dynamic_cast<Player*>(it->second.get());
        if (playerPtr) {
            return playerPtr;
        }
    }
    return nullptr;
}

// @todo: Function should be called by server on server side
// @todo: Client should never call this function in production after connecting to server
void GameManager::handleCollisions() {
    for (auto it1 = activeBoats.begin(); it1 != activeBoats.end(); ++it1) {

        for (auto it2 = std::next(it1); it2 != activeBoats.end(); ++it2) {

            Boat* b1 = it1->second.get();
            Boat* b2 = it2->second.get();

            sf::Vector2f pos1 = b1->getPosition();
            sf::Vector2f pos2 = b2->getPosition();

            float dx = pos2.x - pos1.x;
            float dy = pos2.y - pos1.y;
            float distanceSquared = dx * dx + dy * dy;

            float minDistance = COLLIDER_RADIUS * 2.f;

            if (distanceSquared < minDistance * minDistance && distanceSquared > 0.0001f) {

                float distance = std::sqrt(distanceSquared);
                float overlap = minDistance - distance;

                sf::Vector2f pushDirection(dx / distance, dy / distance);

                float repulsionFactor = 5.f;
                sf::Vector2f pushForce = pushDirection * overlap * repulsionFactor;

                b1->addExternalForce(-pushForce);
                b2->addExternalForce(pushForce);
            }
        }
    }
}