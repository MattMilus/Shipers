//
// Created by Wiktor on 14.03.2026.
//

#ifndef SHIPERS_STATEMANAGER_H
#define SHIPERS_STATEMANAGER_H

#include "../managers/GameManager.h"

void player_ready(char* buffer, int sock, struct sockaddr_in* client_addr, GameState* gameState);
void movePlayer(char* buffer, int sock, struct sockaddr_in* client_addr, GameState* gameState);
void broadcast_state(GameState* state);
void player_finished(const GameState* state, int player_id, int points, float race_time);

#endif //SHIPERS_STATEMANAGER_H
