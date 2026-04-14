//
// Created by Wiktor on 14.03.2026.
//

#include "PacketHandler.h"

#include <cstring>
#include <iostream>
#include "../Terminal.h"

#include "../../ServerPackets.h"


void PacketHandler::handleIncomingPacket(char* buffer, std::size_t receivedSize, GameManager* gameManager) {
    if (receivedSize < sizeof(MsgHeader)) {
        std::cerr << "Rejecting packet, too short (" << receivedSize << " bytes).\n";
        return;
    }

    auto* header = reinterpret_cast<MsgHeader*>(buffer);

    switch (header->type) {

        case MSG_GAME_START: {
            gameManager->startGame(buffer, receivedSize);
        }
        case MSG_GAME_STATE: {
            printAt(0, 3, "Received MSG_GAME_STATE\n");
            if (receivedSize == sizeof(PacketGameState)) {
                PacketGameState statePacket;
                std::memcpy(&statePacket, buffer, sizeof(PacketGameState));

                for (int i = 0; i < statePacket.active_players_count; i++) {
                    printAt(0, 4 + i, "player %d on pos %f, %f", i, statePacket.players[i].x, statePacket.players[i].y);
                    int remoteId = statePacket.players[i].player_id;

                    if (!gameManager->hasBoat(remoteId)) {
                        gameManager->addBoat(remoteId, sf::Vector2f(statePacket.players[i].x, statePacket.players[i].y));
                    }

                    Boat* remoteBoat = gameManager->getBoatById(remoteId);

                    remoteBoat->setTargetPosition(sf::Vector2f(statePacket.players[i].x, statePacket.players[i].y));
                    remoteBoat->setTargetAngle(statePacket.players[i].currentAngle);
                    remoteBoat->setRotation(statePacket.players[i].rotation);
                    remoteBoat->setThrottle(statePacket.players[i].throttle);
                }
            }
            break;
        }
        case MSG_PLAYER_DISCONNECTED: {
            if (receivedSize == sizeof(PacketPlayerDisconnected)) {
                PacketPlayerDisconnected disconnectPacket;
                std::memcpy(&disconnectPacket, buffer, sizeof(PacketPlayerDisconnected));

                gameManager->removeBoat(disconnectPacket.player_id);
            }
            break;
        }
        case MSG_TIMEOUT: {
            if (receivedSize == sizeof(PacketTimeout)) {
                PacketTimeout timeoutPacket;
                std::memcpy(&timeoutPacket, buffer, sizeof(PacketTimeout));

                gameManager->removeBoat(timeoutPacket.player_id);
            }
            break;
        }

        default:
            std::cerr << "Received unknown message: " << header->type << "\n";
            break;
    }
}
