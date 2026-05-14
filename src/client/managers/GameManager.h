//
// Created by Wiktor on 12.03.2026.
//

#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include <chrono>
#include <cstdint>
#include <map>
#include <memory>
#include <set>
#include <string>

#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/UdpSocket.hpp"

#include "../entities/Boat.h"
#include "../entities/Player.h"

enum class SessionPhase {
    Lobby,
    Countdown,
    Race
};

struct LobbyPlayerInfo {
    std::string nickname;
};

class GameManager {
private:
    std::map<int, std::unique_ptr<Boat>> activeBoats;
    std::map<int, LobbyPlayerInfo> lobbyPlayers;
    std::set<int> readyPlayers;
    int playerId;
    std::string nickname;
    SessionPhase sessionPhase;
    std::chrono::steady_clock::time_point scheduledStartAt;
    bool connectedToServer;

    sf::UdpSocket* updSocket;
    sf::IpAddress serverIpAddress;

    int addPlayer(int id, sf::Vector2f startPos);
    static sf::Vector2f raceSpawnForId(int id);

public:
    GameManager();

    int connectToServer(const std::string& serverIp, const std::string& nickname);
    void startGame(char* buffer, std::size_t receivedSize);
    int disconnectFromServer();
    void resetConnection();
    void updateSessionState();
    void enterLobby();
    void scheduleRaceStart(std::uint32_t countdownMs);
    bool shouldSendMoves() const;
    bool isReady() const;
    bool isPlayerReady(int id) const;
    void markPlayerReady(int id);
    void clearPlayerReady(int id);
    void setNickname(std::string newNickname);
    const std::string& getNickname() const;
    SessionPhase getSessionPhase() const;
    float getCountdownSecondsLeft() const;
    bool isConnectedToServer() const;
    void setPlayerId(int id);
    void clearLobbyPlayers();
    void upsertLobbyPlayer(int id, std::string nickname);
    const std::map<int, LobbyPlayerInfo>& getLobbyPlayers() const;

    void handleCollisions();

    void setUdpSocket(sf::UdpSocket* socket);
    sf::UdpSocket* getUdpSocket() const;
    void setServerIpAddress(sf::IpAddress ip);
    sf::IpAddress getServerIpAddress() const;
    int addBoat(int id, sf::Vector2f startPos);
    bool hasBoat(int id);
    void removeBoat(int id);
    [[nodiscard]] const std::map<int, std::unique_ptr<Boat>>& getActiveBoats() const;
    [[nodiscard]] Boat* getBoatById(int id) const;
    [[nodiscard]] Player* getPlayer() const;

    int getPlayerId() const;
};

#endif //GAMEMANAGER_H
