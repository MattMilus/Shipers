#include "PacketHandler.h"

#include <cstring>
#include <cmath>
#include <iostream>

#include "../../ServerPackets.h"
#include "ConnectionManager.h"

namespace {
constexpr float LOCAL_POSITION_CORRECTION_DISTANCE = 8.0f;
constexpr float LOCAL_POSITION_SNAP_DISTANCE = 80.0f;
constexpr float LOCAL_POSITION_CORRECTION_FACTOR = 0.2f;
constexpr float LOCAL_VELOCITY_CORRECTION_SPEED = 20.0f;
constexpr float LOCAL_VELOCITY_SNAP_SPEED = 160.0f;
constexpr float LOCAL_VELOCITY_CORRECTION_FACTOR = 0.25f;
constexpr float LOCAL_ANGLE_CORRECTION_DEGREES = 3.0f;
constexpr float LOCAL_ANGLE_SNAP_DEGREES = 45.0f;
constexpr float LOCAL_ANGLE_CORRECTION_FACTOR = 0.25f;

float lengthSquared(const sf::Vector2f& vector) {
    return (vector.x * vector.x) + (vector.y * vector.y);
}

float normalizeAngleDifference(float targetAngle, float currentAngle) {
    float angleDiff = targetAngle - currentAngle;
    while (angleDiff > 180.0f) angleDiff -= 360.0f;
    while (angleDiff < -180.0f) angleDiff += 360.0f;
    return angleDiff;
}

void reconcileLocalBoat(Boat& localBoat, const PlayerSnapshot& snapshot) {
    const sf::Vector2f serverPosition(snapshot.x, snapshot.y);
    const sf::Vector2f serverVelocity(snapshot.velocityX, snapshot.velocityY);

    localBoat.setTargetPosition(serverPosition);
    localBoat.setTargetVelocity(serverVelocity);
    localBoat.setTargetAngle(snapshot.currentAngle);

    const sf::Vector2f currentPosition = localBoat.getPosition();
    const sf::Vector2f positionError = serverPosition - currentPosition;
    const float positionErrorSquared = lengthSquared(positionError);

    if (positionErrorSquared > LOCAL_POSITION_SNAP_DISTANCE * LOCAL_POSITION_SNAP_DISTANCE) {
        localBoat.setPosition(serverPosition);
    } else if (positionErrorSquared > LOCAL_POSITION_CORRECTION_DISTANCE * LOCAL_POSITION_CORRECTION_DISTANCE) {
        localBoat.setPosition(currentPosition + (positionError * LOCAL_POSITION_CORRECTION_FACTOR));
    }

    const sf::Vector2f currentVelocity = localBoat.getVelocity();
    const sf::Vector2f velocityError = serverVelocity - currentVelocity;
    const float velocityErrorSquared = lengthSquared(velocityError);

    if (velocityErrorSquared > LOCAL_VELOCITY_SNAP_SPEED * LOCAL_VELOCITY_SNAP_SPEED) {
        localBoat.setVelocity(serverVelocity);
    } else if (velocityErrorSquared > LOCAL_VELOCITY_CORRECTION_SPEED * LOCAL_VELOCITY_CORRECTION_SPEED) {
        localBoat.setVelocity(currentVelocity + (velocityError * LOCAL_VELOCITY_CORRECTION_FACTOR));
    }

    const float currentAngle = localBoat.getCurrentAngle();
    const float angleDiff = normalizeAngleDifference(snapshot.currentAngle, currentAngle);

    if (std::fabs(angleDiff) > LOCAL_ANGLE_SNAP_DEGREES) {
        localBoat.setCurrentAngle(snapshot.currentAngle);
    } else if (std::fabs(angleDiff) > LOCAL_ANGLE_CORRECTION_DEGREES) {
        localBoat.setCurrentAngle(currentAngle + (angleDiff * LOCAL_ANGLE_CORRECTION_FACTOR));
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
        case MSG_GAME_START: {
            gameManager->startGame(buffer, receivedSize);
            break;
        }
        case MSG_GAME_STATE: {
            if (receivedSize != sizeof(PacketGameState)) {
                break;
            }

            PacketGameState statePacket{};
            std::memcpy(&statePacket, buffer, sizeof(PacketGameState));

            for (int i = 0; i < statePacket.active_players_count; i++) {
                const PlayerSnapshot& snapshot = statePacket.players[i];
                const int remoteId = snapshot.player_id;

                if (remoteId == gameManager->getPlayerId()) {
                    Boat* localBoat = gameManager->getBoatById(remoteId);
                    if (localBoat != nullptr) {
                        reconcileLocalBoat(*localBoat, snapshot);
                        // Ensure local player's points are also synchronized from server
                        localBoat->setPoints(snapshot.points);
                    }
                    continue;
                }

                if (!gameManager->hasBoat(remoteId)) {
                    gameManager->addBoat(remoteId, sf::Vector2f(snapshot.x, snapshot.y));
                }

                Boat* remoteBoat = gameManager->getBoatById(remoteId);
                if (remoteBoat == nullptr) {
                    continue;
                }

                remoteBoat->setTargetPosition(sf::Vector2f(snapshot.x, snapshot.y));
                remoteBoat->setTargetAngle(snapshot.currentAngle);
                remoteBoat->setTargetVelocity(sf::Vector2f(snapshot.velocityX, snapshot.velocityY));
                remoteBoat->setRotation(snapshot.rotation);
                remoteBoat->setThrottle(snapshot.throttle);
                // Update points immediately so UI/debug reflects coin collection in real-time
                remoteBoat->setPoints(snapshot.points);
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
            ConnectionManager::loadLobbySnapshot(gameManager, lobbyPacket);
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
        case MSG_PLAYER_FINISHED: {
            if (receivedSize != sizeof(PacketPlayerFinished)) {
                break;
            }

            PacketPlayerFinished finishedPacket{};
            std::memcpy(&finishedPacket, buffer, sizeof(PacketPlayerFinished));

            Boat* player = gameManager->getBoatById(finishedPacket.player_id);
            if (player == nullptr) {
                return;
            }

            gameManager->setPlayerTime(finishedPacket.player_id, finishedPacket.time);
            player->setFinished(true);
            player->addPoints(finishedPacket.finishingPoints);

            if (finishedPacket.player_id == gameManager->getPlayerId()) {
                gameManager->localPlayerFinished();
            }
            break;
        }
        case MSG_COINS_STATE: {
            if (receivedSize != sizeof(PacketCoinsState)) {
                break;
            }

            PacketCoinsState coinsPacket{};
            std::memcpy(&coinsPacket, buffer, sizeof(PacketCoinsState));
            gameManager->setCoinsState(coinsPacket.coins_bits);
            break;
        }
        case MSG_COIN_RESPAWN: {
            if (receivedSize != sizeof(PacketCoinRespawn)) break;
            PacketCoinRespawn r;
            std::memcpy(&r, buffer, sizeof(r));

            // update coin position and active state
            int idx = r.coin_index;
            if (idx >= 0 && idx < 64) {
                int g = idx / 8;
                int c = idx % 8;
                auto& coin = const_cast<Coin&>(gameManager->getCoinGroups()[g][c]);
                coin.setPosition({r.x, r.y});
                coin.setActive(true);
                gameManager->setCoinCooldown(idx, r.cooldown_ms);
            }
            break;
        }
        default:
            std::cerr << "Received unknown message: " << header->type << "\n";
            break;
    }
}
