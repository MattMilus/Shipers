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
            gameState->players[i].boat.rotation = move_data->rotation;
            gameState->players[i].boat.throttle = move_data->throttle;

            gameState->players[i].lastActivityTime = time(NULL);
            break;
        }
    }

    pthread_mutex_unlock(&gameState->lock);
}

<<<<<<< HEAD
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
=======
/**
 * Make sure to lock gamestate->lock before calling this function
 * @param state
 */
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

            packet.players[packet.active_players_count] = snapshot;
            packet.active_players_count++;
        }
    }
>>>>>>> fe0a395227799a729b4d41892a4ca4981f9b3b93

    if (packet.active_players_count > 0) {
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (state->players[i].isActive) {
                sendto(state->listenfd_socket, &packet, sizeof(PacketGameState), 0,
                       (struct sockaddr*)&state->players[i].client_addr, sizeof(struct sockaddr_in));
            }
        }
<<<<<<< HEAD

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
=======
    }
    printf("Boat 0: pos x: %f, pos y: %f\n", state->players[0].boat.position.x, state->players[0].boat.position.y);
}
>>>>>>> fe0a395227799a729b4d41892a4ca4981f9b3b93
