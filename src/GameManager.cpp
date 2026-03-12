//
// Created by Wiktor on 12.03.2026.
//

#include "GameManager.h"

#include "entities/Player.h"

GameManager::GameManager() {
    // Initialize any necessary game state here
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

Player* GameManager::getPlayerById(const int id) const {
    auto it = activeBoats.find(id);
    if (it != activeBoats.end()) {
        Player* playerPtr = dynamic_cast<Player*>(it->second.get());
        if (playerPtr) {
            return playerPtr;
        }
    }
    return nullptr;
}
