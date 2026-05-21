//
// Created by Wiktor on 14.03.2026.
//

#include "ConnectionManager.h"

#include <cstring>
#include <iostream>
#include <optional>
#include <string>

#include "../GameManager.h"
#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/Socket.hpp"
#include "SFML/Network/UdpSocket.hpp"
#include "SFML/System/Clock.hpp"

#include "../../env.h"

void ConnectionManager::loadLobbySnapshot(GameManager* gameManager, const PacketAckJoinLobby& lobbyPacket) {
    gameManager->clearLobbyPlayers();

    for (int i = 0; i < lobbyPacket.active_players_count; ++i) {
        const LobbyPlayerSnapshot& playerSnapshot = lobbyPacket.players[i];
        gameManager->upsertLobbyPlayer(playerSnapshot.player_id, playerSnapshot.nickname);

        if (playerSnapshot.is_ready != 0) {
            gameManager->markPlayerReady(playerSnapshot.player_id);
        }
    }
}

int ConnectionManager::connectToServer(GameManager* gameManager,
                                       const std::string& ipString) {
    const std::optional<sf::IpAddress> serverIp = sf::IpAddress::resolve(ipString);
    if (!serverIp) {
        std::cerr << "Invalid IP address!\n";
        return -1;
    }

    auto* dynamicSocket = new sf::UdpSocket();
    gameManager->setUdpSocket(dynamicSocket);
    gameManager->setServerIpAddress(serverIp.value());

    sf::UdpSocket& socket = *dynamicSocket;

    PacketConnect connectPacket{};
    connectPacket.type = MSG_CONNECT;

    if (socket.send(&connectPacket, sizeof(connectPacket), *serverIp, ENV_SERVER_PORT) != sf::Socket::Status::Done) {
        std::cerr << "Error while connecting to server!\n";
        gameManager->resetConnection();
        return -1;
    }

    socket.setBlocking(false);
    sf::Clock timeoutClock;
    int acceptedPlayerId = -1;

    while (timeoutClock.getElapsedTime().asSeconds() < 5.0f) {
        char buffer[1024];
        std::size_t received = 0;
        std::optional<sf::IpAddress> senderIp;
        unsigned short senderPort = 0;

        if (socket.receive(buffer, sizeof(buffer), received, senderIp, senderPort) != sf::Socket::Status::Done) {
            continue;
        }

        if (received != sizeof(PacketAccepted)) {
            continue;
        }

        PacketAccepted acceptedPacket{};
        std::memcpy(&acceptedPacket, buffer, sizeof(PacketAccepted));
        if (acceptedPacket.type != MSG_ACCEPTED) {
            continue;
        }

        acceptedPlayerId = acceptedPacket.player_id;
        break;
    }

    if (acceptedPlayerId == -1) {
        std::cerr << "No response from the server (timeout). Make sure the server is working.\n";
        gameManager->resetConnection();
        return -1;
    }

    gameManager->setPlayerId(acceptedPlayerId);
    return acceptedPlayerId;
}

int ConnectionManager::joinLobby(GameManager* gameManager, const std::string& nickname) {
    gameManager->setNickname(nickname);

    PacketJoinLobby joinLobbyPacket{};
    joinLobbyPacket.type = MSG_JOIN_LOBBY;
    joinLobbyPacket.player_id = gameManager->getPlayerId();
    std::strncpy(joinLobbyPacket.nickname, nickname.c_str(), sizeof(joinLobbyPacket.nickname) - 1);
    joinLobbyPacket.nickname[sizeof(joinLobbyPacket.nickname) - 1] = '\0';

    sf::UdpSocket* socket = gameManager->getUdpSocket();

    if (socket->send(&joinLobbyPacket, sizeof(joinLobbyPacket), gameManager->getServerIpAddress(), ENV_SERVER_PORT) != sf::Socket::Status::Done) {
        std::cerr << "Error while joining lobby!\n";
        gameManager->resetConnection();
        return -1;
    }

    sf::Clock timeoutClock;
    timeoutClock.restart();

    while (timeoutClock.getElapsedTime().asSeconds() < 5.0f) {
        char buffer[2048];
        std::size_t received = 0;
        std::optional<sf::IpAddress> senderIp;
        unsigned short senderPort = 0;

        if (socket->receive(buffer, sizeof(buffer), received, senderIp, senderPort) != sf::Socket::Status::Done) {
            continue;
        }

        if (received != sizeof(PacketAckJoinLobby)) {
            continue;
        }

        PacketAckJoinLobby lobbyPacket{};
        std::memcpy(&lobbyPacket, buffer, sizeof(PacketAckJoinLobby));
        if (lobbyPacket.type != MSG_ACK_JOIN_LOBBY || lobbyPacket.player_id != gameManager->getPlayerId()) {
            continue;
        }

        loadLobbySnapshot(gameManager, lobbyPacket);
        return gameManager->getPlayerId();
    }

    std::cerr << "Lobby handshake timed out.\n";
    gameManager->resetConnection();
    return -1;
}

int ConnectionManager::disconnectFromServer(GameManager *gameManager) {
    PacketDisconnect disconnectPacket{};
    disconnectPacket.type = MSG_DISCONNECT;
    disconnectPacket.player_id = gameManager->getPlayerId();

    if (gameManager->getUdpSocket()->send(
            &disconnectPacket,
            sizeof(disconnectPacket),
            gameManager->getServerIpAddress(),
            ENV_SERVER_PORT) != sf::Socket::Status::Done) {
        std::cerr << "Error while disconnecting from server!\n";
        return -1;
    }

    return 0;
}
