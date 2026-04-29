//
// Created by Wiktor on 14.03.2026.
//

#ifndef SHIPERS_GAMEMANAGER_H
#define SHIPERS_GAMEMANAGER_H

#include <stddef.h>
#include <stdint.h>
#include <netinet/in.h>
#include <pthread.h>
#include "../entities/Boat.h"

#define MAX_PLAYERS 4
#define GAME_START_COUNTDOWN_MS 5000
#define RACE_START_ANGLE_DEGREES 0.0f

typedef enum {
    GAME_PHASE_LOBBY = 0,
    GAME_PHASE_COUNTDOWN = 1,
    GAME_PHASE_RACE = 2
} GamePhase;

typedef struct {
    int playerId;
    Boat boat;

    int isActive;
    int isReady;
    char nickname[32];

    time_t lastActivityTime;
    struct sockaddr_in client_addr;
} Player;

typedef struct {
    Player players[MAX_PLAYERS];
    int current_player_count;

    int listenfd_socket;
    GamePhase phase;
    uint64_t scheduled_start_ms;

    pthread_mutex_t lock;
} GameState;

void game_manager_init(GameState* state, int listenfd_socket);
// Returns the assigned player id, or -1 when the server is full.
int game_manager_add_player(GameState* state, struct sockaddr_in *client_addr, const char* nickname);
// Returns -1 when the player does not exist, 0 when removed without lobby reset,
// and 1 when the removal cancelled countdown and returned everyone to the lobby.
int game_manager_remove_player(GameState* state, int playerId);
void game_manager_update_activity(GameState* state, int playerId);
// Returns 1 when the player transitioned to ready in the lobby, otherwise 0.
int game_manager_mark_ready(GameState* state, int playerId);
// Returns 1 exactly once, when the server schedules a new countdown.
int game_manager_try_schedule_start(GameState* state);
void game_manager_reset_to_lobby(GameState* state);
void game_manager_broadcast(GameState* state, const void* packet, size_t size);
int game_manager_is_race_active(GameState* state);
int game_manager_has_countdown_expired(GameState* state);
uint32_t game_manager_get_remaining_countdown_ms(GameState* state);

#endif //SHIPERS_GAMEMANAGER_H
