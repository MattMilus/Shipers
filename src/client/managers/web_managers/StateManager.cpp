//
// Created by Wiktor on 14.03.2026.
//

#include "StateManager.h"

#include <iostream>

#include "ServerPackets.h"

void StateManager::sendMoveInformation(GameManager *gameManager) {
    if (Player* localPlayer = gameManager->getPlayer()) {
        Boat* myBoat = localPlayer;

        PacketMove movePacket;
        movePacket.type = MSG_MOVE;
        movePacket.player_id = gameManager->getPlayerId();
        movePacket.x = myBoat->getPosition().x;
        movePacket.y = myBoat->getPosition().y;
        movePacket.currentAngle = myBoat->getCurrentAngle();
        movePacket.angleCommand = myBoat->getAngleCommand();
        movePacket.throttle = myBoat->getThrottle();

        sf::Socket::Status status = gameManager->getUdpSocket()->send(
            &movePacket,
            sizeof(movePacket),
            gameManager->getServerIpAddress(),
            ENV_SERVER_PORT
        );

        if (status != sf::Socket::Status::Done) {
            std::cerr << "Error while sending move packet! Powod: ";

            if (status == sf::Socket::Status::NotReady) {
                std::cerr << "[NotReady] Bufor pelny! Wysylasz pakiety za szybko (brak limitu 30Hz?).\n";
            }
            else if (status == sf::Socket::Status::Error) {
                // To wyciągnie IP jako tekst, żebyśmy zobaczyli, czy nie jest zepsute
                std::cerr << "[Error] Blad gniazda lub zly adres IP! Adres to: "
                          << gameManager->getServerIpAddress().toString() << "\n";
            }
            else if (status == sf::Socket::Status::Disconnected) {
                std::cerr << "[Disconnected] Gniazdo zostalo zamkniete.\n";
            }
            else {
                std::cerr << "Nieznany kod bledu SFML.\n";
            }
        }
    }
}
