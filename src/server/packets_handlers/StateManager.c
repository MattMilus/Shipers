//
// Created by Wiktor on 14.03.2026.
//

#include "StateManager.h"
#include "../ServerPackets.h"

void movePlayer(char* buffer, int sock, struct sockaddr_in *client_addr, GameState *gameState) {
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
