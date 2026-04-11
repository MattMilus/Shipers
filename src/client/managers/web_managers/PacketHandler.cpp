//
// Created by Wiktor on 14.03.2026.
//

#include "PacketHandler.h"

#include <cstring>
#include <iostream>
#include "../Terminal.h"

#include "ServerPackets.h"


void PacketHandler::handleIncomingPacket(char* buffer, std::size_t receivedSize, GameManager* gameManager) {
    if (receivedSize < sizeof(MsgHeader)) {
        std::cerr << "Rejecting packet, too short (" << receivedSize << " bytes).\n";
        return;
    }

    auto* header = reinterpret_cast<MsgHeader*>(buffer);

    switch (header->type) {

        case MSG_GAME_STATE: {
            printAt(0, 3, "Received MSG_GAME_STATE\n");
            if (receivedSize == sizeof(PacketGameState)) {
                PacketGameState statePacket;
                std::memcpy(&statePacket, buffer, sizeof(PacketGameState));

                for (int i = 0; i < statePacket.active_players_count; i++) {
                    printAt(0, 4 + i, "player %d on pos %f, %f", i, statePacket.players[i].x, statePacket.players[i].y);
                    int remoteId = statePacket.players[i].player_id;

                    if (remoteId != gameManager->getPlayerId()) {
                        if (!gameManager->hasBoat(remoteId)) {
                            gameManager->addBoat(remoteId, sf::Vector2f(statePacket.players[i].x, statePacket.players[i].y));
                        }

                        Boat* remoteBoat = gameManager->getBoatById(remoteId);
                        remoteBoat->setPosition(sf::Vector2f(statePacket.players[i].x, statePacket.players[i].y));
                        remoteBoat->setCurrentAngle(statePacket.players[i].currentAngle);
                        remoteBoat->setThrottle(statePacket.players[i].throttle);
                    }
                }
            }
            break;
        }

        // Tutaj w przyszłości możesz dodać np. MSG_PLAYER_DISCONNECTED
        /*
        case MSG_PLAYER_DISCONNECTED: {
            // ...
            break;
        }
        */

        default:
            std::cerr << "Received unknown message: " << header->type << "\n";
            break;
    }
}
