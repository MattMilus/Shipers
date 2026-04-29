#include "StateManager.h"

#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "../ServerPackets.h"

void player_ready(char* buffer, int sock, struct sockaddr_in *client_addr, GameState *gameState) {
    (void)sock;
    (void)client_addr;

    PacketPlayerReady *readyPacket = (PacketPlayerReady *)buffer;

    if (!game_manager_mark_ready(gameState, readyPacket->player_id)) {
        return;
    }

    PacketAckReady ackPacket;
    ackPacket.type = MSG_ACK_READY;
    ackPacket.player_id = readyPacket->player_id;
    game_manager_broadcast(gameState, &ackPacket, sizeof(ackPacket));
}

void movePlayer(char* buffer, int sock, struct sockaddr_in *client_addr, GameState *gameState) {
    (void)sock;
    (void)client_addr;

    if (!game_manager_is_race_active(gameState)) {
        return;
    }

    PacketMove *move_data = (PacketMove *)buffer;

    pthread_mutex_lock(&gameState->lock);

    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (gameState->players[i].isActive && gameState->players[i].playerId == move_data->player_id) {
            gameState->players[i].boat.position.x = move_data->x;
            gameState->players[i].boat.position.y = move_data->y;
            gameState->players[i].boat.current_angle = move_data->currentAngle;
            gameState->players[i].boat.angle_command = move_data->angleCommand;
            gameState->players[i].boat.throttle = move_data->throttle;

            gameState->players[i].lastActivityTime = time(NULL);
            break;
        }
    }

    pthread_mutex_unlock(&gameState->lock);
}

void* state_broadcaster(void* arg) {
    GameState* state = (GameState*)arg;
    const int tick_rate_microseconds = 33333;

    for (;;) {
        usleep(tick_rate_microseconds);

        if (game_manager_try_schedule_start(state)) {
            PacketGameScheduledStart scheduledPacket;
            scheduledPacket.type = MSG_GAME_SCHEDULED_START;
            scheduledPacket.countdown_ms = GAME_START_COUNTDOWN_MS;
            game_manager_broadcast(state, &scheduledPacket, sizeof(scheduledPacket));
        }

        game_manager_has_countdown_expired(state);

        if (!game_manager_is_race_active(state)) {
            continue;
        }

        PacketGameState packet;
        packet.type = MSG_GAME_STATE;
        packet.active_players_count = 0;

        pthread_mutex_lock(&state->lock);

        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (state->players[i].isActive) {
                PlayerSnapshot snapshot;
                snapshot.player_id = state->players[i].playerId;
                snapshot.x = state->players[i].boat.position.x;
                snapshot.y = state->players[i].boat.position.y;
                snapshot.currentAngle = state->players[i].boat.current_angle;
                snapshot.angleCommand = state->players[i].boat.angle_command;
                snapshot.throttle = state->players[i].boat.throttle;

                packet.players[packet.active_players_count] = snapshot;
                packet.active_players_count++;
            }
        }

        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (state->players[i].isActive) {
                sendto(state->listenfd_socket, &packet, sizeof(packet), 0,
                       (struct sockaddr*)&state->players[i].client_addr, sizeof(struct sockaddr_in));
            }
        }

        pthread_mutex_unlock(&state->lock);
    }

    return NULL;
}
