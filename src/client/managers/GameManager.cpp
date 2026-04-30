#include "GameManager.h"

#include <algorithm>
#include <cmath>
<<<<<<< HEAD
#include <utility>
=======
#include <cstring>
#include <bits/fs_fwd.h>
>>>>>>> fe0a395227799a729b4d41892a4ca4981f9b3b93

#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/Socket.hpp"
#include "web_managers/ConnectionManager.h"
#include "../ServerPackets.h"

namespace {
std::string fallbackNicknameForId(const int id) {
    return "Player" + std::to_string(id);
}
}

GameManager::GameManager()
    : playerId(-1),
      nickname("Player"),
      sessionPhase(SessionPhase::Lobby),
      scheduledStartAt(std::chrono::steady_clock::now()),
      connectedToServer(false),
      updSocket(nullptr),
      serverIpAddress(sf::IpAddress::resolve("127.0.0.1").value()) {
}

sf::Vector2f GameManager::raceSpawnForId(const int id) {
    return {300.0f + (static_cast<float>(id) * 90.0f), 450.0f};
}

int GameManager::connectToServer(const std::string& serverIp, const std::string& newNickname) {
    const int id = ConnectionManager::connectToServer(this, serverIp, newNickname);
    if (id == -1) {
        return -1;
    }

<<<<<<< HEAD
    connectedToServer = true;
    sessionPhase = SessionPhase::Lobby;
    scheduledStartAt = std::chrono::steady_clock::now();

    if (!hasBoat(playerId)) {
        addPlayer(playerId, raceSpawnForId(playerId));
    }

    for (const auto& [idToSync, _] : lobbyPlayers) {
        if (idToSync == playerId || hasBoat(idToSync)) {
            continue;
        }

        addBoat(idToSync, raceSpawnForId(idToSync));
    }

=======
    playerId = id;
    addPlayer(id, sf::Vector2f(100.f, 300.f));
>>>>>>> fe0a395227799a729b4d41892a4ca4981f9b3b93
    return playerId;
}

void GameManager::startGame(char* buffer, std::size_t receivedSize) {
    printf("Received start game");
    if (receivedSize == sizeof(PacketGameStart)) {
        printf("Received start game");
        PacketGameStart gameStartPacket;
        std::memcpy(&gameStartPacket, buffer, sizeof(PacketGameState));

        for (int i = 0; i < gameStartPacket.active_players_count; i++) {
            int remoteId = gameStartPacket.players[i].player_id;

            if (remoteId == getPlayerId()) {
                // @Todo : Clear this mess
                removeBoat(remoteId);
                addPlayer(remoteId, sf::Vector2f(gameStartPacket.players[i].x, gameStartPacket.players[i].y));
            } else {
                addBoat(remoteId, sf::Vector2f(gameStartPacket.players[i].x, gameStartPacket.players[i].y));
            }
        }
    }
}

int GameManager::disconnectFromServer() {
    if (!connectedToServer || updSocket == nullptr || playerId < 0) {
        resetConnection();
        return 0;
    }

    const int disconnectStatus = ConnectionManager::disconnectFromServer(this);
    resetConnection();
    return disconnectStatus;
}

void GameManager::resetConnection() {
    connectedToServer = false;
    playerId = -1;
    sessionPhase = SessionPhase::Lobby;
    scheduledStartAt = std::chrono::steady_clock::now();
    activeBoats.clear();
    clearLobbyPlayers();

    if (updSocket != nullptr) {
        delete updSocket;
        updSocket = nullptr;
    }
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
}

void GameManager::scheduleRaceStart(const std::uint32_t countdownMs) {
    sessionPhase = SessionPhase::Countdown;
    scheduledStartAt = std::chrono::steady_clock::now() + std::chrono::milliseconds(countdownMs);

    for (const auto& [idToSync, _] : lobbyPlayers) {
        if (idToSync == playerId) {
            if (!hasBoat(idToSync)) {
                addPlayer(idToSync, raceSpawnForId(idToSync));
            }
            continue;
        }

        if (!hasBoat(idToSync)) {
            addBoat(idToSync, raceSpawnForId(idToSync));
        }
    }

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
    return connectedToServer && sessionPhase == SessionPhase::Race;
}

bool GameManager::isReady() const {
    if (playerId < 0) {
        return false;
    }

    return readyPlayers.find(playerId) != readyPlayers.end();
}

bool GameManager::isPlayerReady(const int id) const {
    return readyPlayers.find(id) != readyPlayers.end();
}

void GameManager::markPlayerReady(const int id) {
    readyPlayers.insert(id);
}

void GameManager::clearPlayerReady(const int id) {
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

bool GameManager::isConnectedToServer() const {
    return connectedToServer;
}

void GameManager::setPlayerId(const int id) {
    playerId = id;
}

void GameManager::clearLobbyPlayers() {
    lobbyPlayers.clear();
    readyPlayers.clear();
}

void GameManager::upsertLobbyPlayer(const int id, std::string playerNickname) {
    if (playerNickname.empty()) {
        playerNickname = fallbackNicknameForId(id);
    }

    lobbyPlayers[id] = LobbyPlayerInfo{std::move(playerNickname)};
}

const std::map<int, LobbyPlayerInfo>& GameManager::getLobbyPlayers() const {
    return lobbyPlayers;
}

int GameManager::addPlayer(const int id, const sf::Vector2f startPos) {
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

void GameManager::setServerIpAddress(const sf::IpAddress ip) {
    serverIpAddress = ip;
}

sf::IpAddress GameManager::getServerIpAddress() const {
    return serverIpAddress;
}

int GameManager::addBoat(const int id, const sf::Vector2f startPos) {
    if (activeBoats.size() >= 4) {
        return -1;
    }

    activeBoats[id] = std::make_unique<Boat>(startPos);
    return id;
}

bool GameManager::hasBoat(const int id) {
    return activeBoats.find(id) != activeBoats.end();
}

void GameManager::removeBoat(const int id) {
    readyPlayers.erase(id);
    lobbyPlayers.erase(id);
    activeBoats.erase(id);
}

const std::map<int, std::unique_ptr<Boat>>& GameManager::getActiveBoats() const {
    return activeBoats;
}

Boat* GameManager::getBoatById(const int id) const {
    const auto it = activeBoats.find(id);
    if (it != activeBoats.end()) {
        return it->second.get();
    }
    return nullptr;
}

Player* GameManager::getPlayer() const {
    const auto it = activeBoats.find(playerId);
    if (it != activeBoats.end()) {
        return dynamic_cast<Player*>(it->second.get());
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

            const sf::Vector2f pos1 = b1->getPosition();
            const sf::Vector2f pos2 = b2->getPosition();

            const float dx = pos2.x - pos1.x;
            const float dy = pos2.y - pos1.y;
            const float distanceSquared = dx * dx + dy * dy;

            const float minDistance = COLLIDER_RADIUS * 2.f;

            if (distanceSquared < minDistance * minDistance && distanceSquared > 0.0001f) {
                const float distance = std::sqrt(distanceSquared);
                const float overlap = minDistance - distance;

                const sf::Vector2f pushDirection(dx / distance, dy / distance);
                const float repulsionFactor = 5.f;
                const sf::Vector2f pushForce = pushDirection * overlap * repulsionFactor;

                b1->addExternalForce(-pushForce);
                b2->addExternalForce(pushForce);
            }
        }
    }
}
