//
// Created by Wiktor on 14.03.2026.
//

#ifndef SHIPERS_STATEMANAGER_H
#define SHIPERS_STATEMANAGER_H

#include "../managers/GameManager.h"

void player_ready(char* buffer, int sock, struct sockaddr_in *client_addr, GameState *gameState);
void movePlayer(char* buffer, int sock, struct sockaddr_in *client_addr, GameState *gameState);
<<<<<<< HEAD
void* state_broadcaster(void* arg);
=======

void broadcast_state(GameState* state);
>>>>>>> fe0a395227799a729b4d41892a4ca4981f9b3b93

#endif //SHIPERS_STATEMANAGER_H
