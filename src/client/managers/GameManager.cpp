#include "GameManager.h"

#include <algorithm>
#include <cmath>
#include <utility>

#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/Socket.hpp"
#include "web_managers/ConnectionManager.h"

GameManager::GameManager()
    : playerId(0),
      nickname("Player"),
      sessionPhase(SessionPhase::Lobby),
      scheduledStartAt(std::chrono::steady_clock::now()),
      updSocket(nullptr),
      serverIpAddress(sf::IpAddress::resolve("127.0.0.1").value()) {
}

sf::Vector2f GameManager::lobbySpawnForId(int id) {
    return {100.0f, 100.0f + ((std::max(1, id) - 1) * 80.0f)};
}

sf::Vector2f GameManager::raceSpawnForId(int id) {
    return {300.0f + ((std::max(1, id) - 1) * 90.0f), 450.0f};
}

int GameManager::connectToServer() {
    const int id = ConnectionManager::connectToServer(this);
    if (id == -1) {
        return -1;
    }

    playerId = id;
    sessionPhase = SessionPhase::Lobby;
    readyPlayers.clear();
    addPlayer(id, lobbySpawnForId(id));
    return playerId;
}

int GameManager::disconnectFromServer() {
    return ConnectionManager::disconnectFromServer(this);
}

void GameManager::updateSessionState() {
    if (sessionPhase == SessionPhase::Countdown &&
        std::chrono::steady_clock::now() >= scheduledStartAt) {
        sessionPhase = SessionPhase::Race;
    }
}

void GameManager::enterLobby() {
    sessionPhase = SessionPhase::Lobby;
    readyPlayers.clear();
    scheduledStartAt = std::chrono::steady_clock::now();

    for (auto it = activeBoats.begin(); it != activeBoats.end();) {
        if (it->first != playerId) {
            it = activeBoats.erase(it);
            continue;
        }

        it->second->setPosition(lobbySpawnForId(playerId));
        it->second->setTargetPosition(lobbySpawnForId(playerId));
        it->second->setCurrentAngle(0.0f);
        it->second->setTargetAngle(0.0f);
        it->second->setThrottle(0.0f);
        ++it;
    }
}

void GameManager::scheduleRaceStart(std::uint32_t countdownMs) {
    sessionPhase = SessionPhase::Countdown;
    scheduledStartAt = std::chrono::steady_clock::now() + std::chrono::milliseconds(countdownMs);

    for (auto& [id, boat] : activeBoats) {
        const sf::Vector2f spawn = raceSpawnForId(id);
        boat->setPosition(spawn);
        boat->setTargetPosition(spawn);
        boat->setCurrentAngle(0.0f);
        boat->setTargetAngle(0.0f);
        boat->setThrottle(0.0f);
    }
}

bool GameManager::shouldSendMoves() const {
    return sessionPhase == SessionPhase::Race;
}

bool GameManager::isReady() const {
    return readyPlayers.find(playerId) != readyPlayers.end();
}

void GameManager::markPlayerReady(int id) {
    readyPlayers.insert(id);
}

void GameManager::clearPlayerReady(int id) {
    readyPlayers.erase(id);
}

void GameManager::setNickname(std::string newNickname) {
    nickname = std::move(newNickname);
}

const std::string& GameManager::getNickname() const {
    return nickname;
}

SessionPhase GameManager::getSessionPhase() const {
    return sessionPhase;
}

float GameManager::getCountdownSecondsLeft() const {
    if (sessionPhase != SessionPhase::Countdown) {
        return 0.0f;
    }

    const auto now = std::chrono::steady_clock::now();
    if (now >= scheduledStartAt) {
        return 0.0f;
    }

    return std::chrono::duration<float>(scheduledStartAt - now).count();
}

int GameManager::addPlayer(int id, sf::Vector2f startPos) {
    if (activeBoats.size() >= 4) {
        return -1;
    }

    activeBoats[id] = std::make_unique<Player>(id, startPos);
    return id;
}

void GameManager::setUdpSocket(sf::UdpSocket *socket) {
    updSocket = socket;
}

sf::UdpSocket* GameManager::getUdpSocket() const {
    return updSocket;
}

void GameManager::setServerIpAddress(sf::IpAddress ip) {
    serverIpAddress = ip;
}

sf::IpAddress GameManager::getServerIpAddress() const {
    return serverIpAddress;
}

int GameManager::addBoat(int id, sf::Vector2f startPos) {
    if (activeBoats.size() >= 4) {
        return -1;
    }

    activeBoats[id] = std::make_unique<Boat>(startPos);
    return id;
}

bool GameManager::hasBoat(const int id) {
    return activeBoats.find(id) != activeBoats.end();
}

void GameManager::removeBoat(int id) {
    readyPlayers.erase(id);
    activeBoats.erase(id);
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

int GameManager::getPlayerId() const {
    return playerId;
}

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
