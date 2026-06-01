//
// Created by Wiktor on 14.03.2026.
//

#ifndef SHIPERS_GAMEMANAGER_H
#define SHIPERS_GAMEMANAGER_H

#include <netinet/in.h>
#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>

#include "../entities/Boat.h"
#include "../entities/Track.h"
#include "../entities/Coin.h"

#define MAX_PLAYERS 4
#define GAME_START_COUNTDOWN_MS 5000
#define RACE_START_ANGLE_DEGREES 0.0f
#define WINNING_POINTS 1000
#define POINTS_LOSS_FOR_SECOND 50

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
    int isFinished;
    char nickname[32];

    time_t lastActivityTime;
    struct sockaddr_in client_addr;
} Player;

typedef struct {
    Player players[MAX_PLAYERS];
    int current_player_count;

    // Coins: 8 groups * 8 coins = 64
    Coin coins[64];
    uint64_t coins_bits; // bit == 1 -> collected

    int listenfd_socket;
    GamePhase phase;
    uint64_t scheduled_start_ms;
    uint64_t race_start_ms;
    uint64_t winner_time;

    int player_finished_count;

    pthread_mutex_t lock;
} GameState;

uint64_t now_ms(void);
void game_manager_init(GameState* state, int listenfd_socket);
int game_manager_add_player(GameState* state, struct sockaddr_in* client_addr, const char* nickname);
int game_manager_remove_player(GameState* state, int playerId);
void game_manager_update_activity(GameState* state, int playerId);
int game_manager_mark_ready(GameState* state, int playerId);
int game_manager_try_schedule_start(GameState* state);
void game_manager_reset_to_lobby(GameState* state);
void game_manager_broadcast(GameState* state, const void* packet, size_t size);
int game_manager_is_race_active(GameState* state);
int game_manager_has_countdown_expired(GameState* state);
uint32_t game_manager_get_remaining_countdown_ms(GameState* state);
void game_manager_resolve_collisions(GameState* state);

int get_finish_points(GameState* state, uint64_t finishing_time);

#endif //SHIPERS_GAMEMANAGER_H
