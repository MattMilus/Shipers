//
// Created by Wiktor on 14.03.2026.
//

#ifndef SHIPERS_STATEMANAGER_H
#define SHIPERS_STATEMANAGER_H

#include "../managers/GameManager.h"

void movePlayer(char* buffer, int sock, struct sockaddr_in *client_addr, GameState *gameState);

void* timeout_checker(void* arg);
void* state_broadcaster(void* arg);

#endif //SHIPERS_STATEMANAGER_H