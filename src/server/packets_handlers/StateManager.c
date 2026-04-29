//
// Created by Wiktor on 14.03.2026.
//

#include "StateManager.h"

#include <stdio.h>
#include <unistd.h>

#include "../ServerPackets.h"

void movePlayer(char* buffer, int sock, struct sockaddr_in *client_addr, GameState *gameState) {
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

    if (packet.active_players_count > 0) {
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (state->players[i].isActive) {
                sendto(state->listenfd_socket, &packet, sizeof(PacketGameState), 0,
                       (struct sockaddr*)&state->players[i].client_addr, sizeof(struct sockaddr_in));
            }
        }
    }
    printf("Boat 0: pos x: %f, pos y: %f\n", state->players[0].boat.position.x, state->players[0].boat.position.y);
}