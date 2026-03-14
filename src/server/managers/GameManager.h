//
// Created by Wiktor on 14.03.2026.
//

#ifndef SHIPERS_GAMEMANAGER_H
#define SHIPERS_GAMEMANAGER_H

#include <netinet/in.h>
#include <pthread.h>
#include "../entities/Boat.h"

#define MAX_PLAYERS 4

typedef struct {
    int playerId;
    Boat boat;

    int isActive;

    time_t lastActivityTime;
    struct sockaddr_in client_addr;
} Player;

typedef struct {
    Player players[MAX_PLAYERS];
    int current_player_count;

    int listenfd_socket;

    // Semaphores and mutexes
    pthread_mutex_t lock;
} GameState;

void game_manager_init(GameState* state, int listenfd_socket);
int game_manager_add_player(GameState* state, struct sockaddr_in *client_addr);
void game_manager_remove_player(GameState* state, int playerId);
void game_manager_update_activity(GameState* state, int playerId);

#endif //SHIPERS_GAMEMANAGER_H