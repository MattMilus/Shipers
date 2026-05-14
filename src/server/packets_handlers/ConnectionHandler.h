//
// Created by Wiktor on 14.03.2026.
//

#ifndef SHIPERS_CONNECTHANDLER_H
#define SHIPERS_CONNECTHANDLER_H

#include "../managers/GameManager.h"

#define TIMEOUT_SECONDS 5

void accept_connection(char* buffer, int sock, struct sockaddr_in *client_addr, GameState *gameState);
void player_join_lobby(char* buffer, int sock, struct sockaddr_in *client_addr, GameState *gameState);
void player_disconnect(char* buffer, int sock, struct sockaddr_in *client_addr, GameState *gameState);
void* timeout_checker(void* arg);

#endif //SHIPERS_CONNECTHANDLER_H
