#include "PacketHandler.h"

#include <cstring>
#include <iostream>

<<<<<<< HEAD
#include "../Terminal.h"
#include "ServerPackets.h"
=======
#include "../../ServerPackets.h"
>>>>>>> fe0a395227799a729b4d41892a4ca4981f9b3b93

namespace {
void loadLobbySnapshot(GameManager* gameManager, const PacketAckJoinLobby& lobbyPacket) {
    gameManager->clearLobbyPlayers();

    for (int i = 0; i < lobbyPacket.active_players_count; ++i) {
        const LobbyPlayerSnapshot& playerSnapshot = lobbyPacket.players[i];
        gameManager->upsertLobbyPlayer(playerSnapshot.player_id, playerSnapshot.nickname);

        if (playerSnapshot.is_ready != 0) {
            gameManager->markPlayerReady(playerSnapshot.player_id);
        }
    }
}
}

void PacketHandler::handleIncomingPacket(char* buffer, const std::size_t receivedSize, GameManager* gameManager) {
    if (receivedSize < sizeof(MsgHeader)) {
        std::cerr << "Rejecting packet, too short (" << receivedSize << " bytes).\n";
        return;
    }

    auto* header = reinterpret_cast<MsgHeader*>(buffer);

    switch (header->type) {
<<<<<<< HEAD
=======

        case MSG_GAME_START: {
            gameManager->startGame(buffer, receivedSize);
        }
>>>>>>> fe0a395227799a729b4d41892a4ca4981f9b3b93
        case MSG_GAME_STATE: {
            if (receivedSize != sizeof(PacketGameState)) {
                break;
            }

            PacketGameState statePacket{};
            std::memcpy(&statePacket, buffer, sizeof(PacketGameState));

<<<<<<< HEAD
            for (int i = 0; i < statePacket.active_players_count; i++) {
                const int remoteId = statePacket.players[i].player_id;
                printAt(0, 3 + i, "player %d on pos %f, %f", remoteId, statePacket.players[i].x, statePacket.players[i].y);

                if (remoteId == gameManager->getPlayerId()) {
                    continue;
=======
                    if (!gameManager->hasBoat(remoteId)) {
                        gameManager->addBoat(remoteId, sf::Vector2f(statePacket.players[i].x, statePacket.players[i].y));
                    }

                    Boat* remoteBoat = gameManager->getBoatById(remoteId);

                    remoteBoat->setTargetPosition(sf::Vector2f(statePacket.players[i].x, statePacket.players[i].y));
                    remoteBoat->setTargetAngle(statePacket.players[i].currentAngle);
                    remoteBoat->setTargetVelocity(sf::Vector2f(statePacket.players[i].velocityX, statePacket.players[i].velocityY));
                    remoteBoat->setRotation(statePacket.players[i].rotation);
                    remoteBoat->setThrottle(statePacket.players[i].throttle);
>>>>>>> fe0a395227799a729b4d41892a4ca4981f9b3b93
                }

                if (!gameManager->hasBoat(remoteId)) {
                    gameManager->addBoat(remoteId, sf::Vector2f(statePacket.players[i].x, statePacket.players[i].y));
                }

                Boat* remoteBoat = gameManager->getBoatById(remoteId);
                remoteBoat->setTargetPosition(sf::Vector2f(statePacket.players[i].x, statePacket.players[i].y));
                remoteBoat->setTargetAngle(statePacket.players[i].currentAngle);
                remoteBoat->setThrottle(statePacket.players[i].throttle);
            }
            break;
        }
        case MSG_PLAYER_DISCONNECTED: {
            if (receivedSize != sizeof(PacketPlayerDisconnected)) {
                break;
            }

            PacketPlayerDisconnected disconnectPacket{};
            std::memcpy(&disconnectPacket, buffer, sizeof(PacketPlayerDisconnected));

            if (disconnectPacket.player_id == gameManager->getPlayerId()) {
                gameManager->resetConnection();
            } else {
                gameManager->removeBoat(disconnectPacket.player_id);
            }
            break;
        }
        case MSG_TIMEOUT: {
            if (receivedSize != sizeof(PacketTimeout)) {
                break;
            }

            PacketTimeout timeoutPacket{};
            std::memcpy(&timeoutPacket, buffer, sizeof(PacketTimeout));

            if (timeoutPacket.player_id == gameManager->getPlayerId()) {
                gameManager->resetConnection();
            } else {
                gameManager->removeBoat(timeoutPacket.player_id);
            }
            break;
        }
        case MSG_ACK_JOIN_LOBBY: {
            if (receivedSize != sizeof(PacketAckJoinLobby)) {
                break;
            }

            PacketAckJoinLobby lobbyPacket{};
            std::memcpy(&lobbyPacket, buffer, sizeof(PacketAckJoinLobby));
            loadLobbySnapshot(gameManager, lobbyPacket);
            break;
        }
        case MSG_NEW_PLAYER_JOIN: {
            if (receivedSize != sizeof(PacketNewPlayerJoin)) {
                break;
            }

            PacketNewPlayerJoin joinPacket{};
            std::memcpy(&joinPacket, buffer, sizeof(PacketNewPlayerJoin));

            gameManager->upsertLobbyPlayer(joinPacket.player_id, joinPacket.nickname);
            if (joinPacket.player_id != gameManager->getPlayerId() && !gameManager->hasBoat(joinPacket.player_id)) {
                gameManager->addBoat(joinPacket.player_id, sf::Vector2f(0.0f, 0.0f));
            }
            break;
        }
        case MSG_ACK_READY: {
            if (receivedSize != sizeof(PacketAckReady)) {
                break;
            }

            PacketAckReady readyPacket{};
            std::memcpy(&readyPacket, buffer, sizeof(PacketAckReady));
            gameManager->markPlayerReady(readyPacket.player_id);
            break;
        }
        case MSG_GAME_SCHEDULED_START: {
            if (receivedSize != sizeof(PacketGameScheduledStart)) {
                break;
            }

            PacketGameScheduledStart scheduledPacket{};
            std::memcpy(&scheduledPacket, buffer, sizeof(PacketGameScheduledStart));
            gameManager->scheduleRaceStart(scheduledPacket.countdown_ms);
            break;
        }
        case MSG_RETURN_TO_LOBBY: {
            gameManager->enterLobby();
            break;
        }
        default:
            std::cerr << "Received unknown message: " << header->type << "\n";
            break;
    }
}
