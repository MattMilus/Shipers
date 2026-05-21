//
// Created by Wiktor on 14.03.2026.
//

#include "StateManager.h"

#include <iostream>

#include "../../ServerPackets.h"

void StateManager::sendReady(GameManager *gameManager) {
    PacketPlayerReady readyPacket;
    readyPacket.type = MSG_PLAYER_READY;
    readyPacket.player_id = gameManager->getPlayerId();

    const sf::Socket::Status status = gameManager->getUdpSocket()->send(
        &readyPacket,
        sizeof(readyPacket),
        gameManager->getServerIpAddress(),
        ENV_SERVER_PORT
    );

    if (status != sf::Socket::Status::Done) {
        std::cerr << "Error while sending ready packet.\n";
    }
}

void StateManager::sendMoveInformation(GameManager *gameManager) {
    if (Player* localPlayer = gameManager->getPlayer()) {
        Boat* myBoat = localPlayer;

        PacketMove movePacket{};
        movePacket.type = MSG_MOVE;
        movePacket.player_id = gameManager->getPlayerId();
        movePacket.x = myBoat->getPosition().x;
        movePacket.y = myBoat->getPosition().y;
        movePacket.currentAngle = myBoat->getCurrentAngle();
        movePacket.rotation = myBoat->getRotation();
        movePacket.throttle = myBoat->getThrottle();

        sf::Socket::Status status = gameManager->getUdpSocket()->send(
            &movePacket,
            sizeof(movePacket),
            gameManager->getServerIpAddress(),
            ENV_SERVER_PORT
        );

        if (status != sf::Socket::Status::Done) {
            std::cerr << "Error while sending move packet! Reason: ";

            if (status == sf::Socket::Status::NotReady) {
                std::cerr << "[NotReady] Buffer full! Sending packets too fast (maybe without 30Hz limit?).\n";
            }
            else if (status == sf::Socket::Status::Error) {
                std::cerr << "[Error] Socket error or invalid ip address: ";
            }
            else if (status == sf::Socket::Status::Disconnected) {
                std::cerr << "[Disconnected] Socket closed.\n";
            }
            else {
                std::cerr << "Unknown SFML error code.\n";
            }
        }
    }
}

void StateManager::sendIAmAlive(GameManager *gameManager) {
    PacketIAmAlive packet;
    packet.type = MSG_I_AM_ALIVE;
    packet.player_id = gameManager->getPlayerId();

    const sf::Socket::Status status = gameManager->getUdpSocket()->send(
        &packet,
        sizeof(packet),
        gameManager->getServerIpAddress(),
        ENV_SERVER_PORT
    );

    if (status != sf::Socket::Status::Done) {
        std::cerr << "Error while sending IAmAlive packet.\n";
    }
}
