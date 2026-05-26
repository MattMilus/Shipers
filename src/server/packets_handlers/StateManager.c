#include "StateManager.h"

#include <sys/socket.h>
#include <time.h>
#include <stdio.h>

#include "../ServerPackets.h"

void player_ready(char* buffer, int sock, struct sockaddr_in* client_addr, GameState* gameState) {
    (void)sock;
    (void)client_addr;

    PacketPlayerReady* readyPacket = (PacketPlayerReady*)buffer;

    if (!game_manager_mark_ready(gameState, readyPacket->player_id)) {
        return;
    }

    PacketAckReady ackPacket;
    ackPacket.type = MSG_ACK_READY;
    ackPacket.player_id = readyPacket->player_id;
    game_manager_broadcast(gameState, &ackPacket, sizeof(ackPacket));
}

void movePlayer(char* buffer, int sock, struct sockaddr_in* client_addr, GameState* gameState) {
    (void)sock;
    (void)client_addr;

    if (!game_manager_is_race_active(gameState)) {
        return;
    }

    PacketMove* move_data = (PacketMove*)buffer;

    pthread_mutex_lock(&gameState->lock);

    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (gameState->players[i].isActive && gameState->players[i].playerId == move_data->player_id) {
            gameState->players[i].boat.rotation = move_data->rotation;
            gameState->players[i].boat.throttle = move_data->throttle;
            gameState->players[i].lastActivityTime = time(NULL);
            break;
        }
    }

    pthread_mutex_unlock(&gameState->lock);
}

void broadcast_state(GameState* state) {
    PacketGameState packet;
    packet.type = MSG_GAME_STATE;
    packet.active_players_count = 0;

    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (state->players[i].isActive) {
            PlayerSnapshot snapshot;
            snapshot.player_id = state->players[i].playerId;
            snapshot.x = state->players[i].boat.position.x;
            snapshot.y = state->players[i].boat.position.y;
            snapshot.currentAngle = state->players[i].boat.current_angle;
            snapshot.rotation = state->players[i].boat.rotation;
            snapshot.throttle = state->players[i].boat.throttle;
            snapshot.velocityX = state->players[i].boat.velocity.x;
            snapshot.velocityY = state->players[i].boat.velocity.y;
            snapshot.points = state->players[i].boat.points;

            packet.players[packet.active_players_count] = snapshot;
            packet.active_players_count++;
        }
    }

    if (packet.active_players_count == 0) {
        return;
    }

    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (state->players[i].isActive) {
            sendto(state->listenfd_socket, &packet, sizeof(packet), 0,
                   (struct sockaddr*)&state->players[i].client_addr, sizeof(struct sockaddr_in));
        }
    }
}

void player_finished(const GameState* state, int player_id, int points, float race_time) {
    PacketPlayerFinished packet;
    packet.type = MSG_PLAYER_FINISHED;
    packet.player_id = player_id;
    packet.place = state->player_finished_count;
    packet.time = race_time;
    packet.finishingPoints = points;

    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (state->players[i].isActive) {
            sendto(state->listenfd_socket, &packet, sizeof(packet), 0,
                   (struct sockaddr*)&state->players[i].client_addr, sizeof(struct sockaddr_in));
        }
    }

    fprintf(stderr, "Player %d finished with %d points\n", player_id, packet.finishingPoints);
    fprintf(stderr, "Player %d finished with time %.2f\n", player_id, race_time);
    fprintf(stderr, "Winner finished with time %.2lu\n", state->winner_time);
    fprintf(stderr, "Race started at time %.2lu\n", state->race_start_ms);
}
