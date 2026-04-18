#include "GameManager.h"

#include <string.h>
#include <sys/socket.h>
#include <time.h>

static uint64_t now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ((uint64_t)ts.tv_sec * 1000ULL) + ((uint64_t)ts.tv_nsec / 1000000ULL);
}

static Vector2f lobby_spawn_for_slot(int slot) {
    Vector2f spawn = {100.0f, 100.0f + (slot * 80.0f)};
    return spawn;
}

static Vector2f race_spawn_for_slot(int slot) {
    Vector2f spawn = {300.0f + (slot * 90.0f), 450.0f};
    return spawn;
}

static void reset_player_to_lobby(Player* player, int slot) {
    Vector2f spawn = lobby_spawn_for_slot(slot);
    boat_init(&player->boat, spawn);
    player->isReady = 0;
}

static void move_player_to_race_start(Player* player, int slot) {
    Vector2f spawn = race_spawn_for_slot(slot);
    boat_init(&player->boat, spawn);
    player->boat.current_angle = 0.0f;
    player->boat.angle_command = 0.0f;
}

static int all_active_players_ready_locked(GameState* state) {
    int active_count = 0;

    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (!state->players[i].isActive) {
            continue;
        }

        active_count++;
        if (!state->players[i].isReady) {
            return 0;
        }
    }

    return active_count >= 2;
}

static void schedule_game_start_locked(GameState* state) {
    state->phase = GAME_PHASE_COUNTDOWN;
    state->scheduled_start_ms = now_ms() + GAME_START_COUNTDOWN_MS;

    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (state->players[i].isActive) {
            move_player_to_race_start(&state->players[i], i);
        }
    }
}

void game_manager_init(GameState* state, int listenfd_socket) {
    pthread_mutex_init(&state->lock, NULL);

    state->current_player_count = 0;
    state->listenfd_socket = listenfd_socket;
    state->phase = GAME_PHASE_LOBBY;
    state->scheduled_start_ms = 0;

    for (int i = 0; i < MAX_PLAYERS; i++) {
        state->players[i].isActive = 0;
        state->players[i].isReady = 0;
        state->players[i].playerId = i + 1;
        state->players[i].nickname[0] = '\0';
        reset_player_to_lobby(&state->players[i], i);
    }
}

int game_manager_add_player(GameState* state, struct sockaddr_in *client_addr, const char* nickname) {
    pthread_mutex_lock(&state->lock);

    if (state->current_player_count >= MAX_PLAYERS) {
        pthread_mutex_unlock(&state->lock);
        return -1;
    }

    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (!state->players[i].isActive) {
            state->players[i].isActive = 1;
            state->players[i].isReady = 0;
            state->current_player_count++;
            state->players[i].client_addr = *client_addr;
            state->players[i].lastActivityTime = time(NULL);
            reset_player_to_lobby(&state->players[i], i);

            if (nickname != NULL) {
                strncpy(state->players[i].nickname, nickname, sizeof(state->players[i].nickname) - 1);
                state->players[i].nickname[sizeof(state->players[i].nickname) - 1] = '\0';
            } else {
                state->players[i].nickname[0] = '\0';
            }

            pthread_mutex_unlock(&state->lock);
            return state->players[i].playerId;
        }
    }

    pthread_mutex_unlock(&state->lock);
    return -1;
}

int game_manager_remove_player(GameState* state, int playerId) {
    int returned_to_lobby = 0;

    pthread_mutex_lock(&state->lock);

    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (state->players[i].playerId == playerId && state->players[i].isActive) {
            state->players[i].isActive = 0;
            state->current_player_count--;
            reset_player_to_lobby(&state->players[i], i);
            state->players[i].nickname[0] = '\0';
            break;
        }
    }

    if (state->current_player_count == 0) {
        state->phase = GAME_PHASE_LOBBY;
        state->scheduled_start_ms = 0;
        returned_to_lobby = 1;
    } else if (state->phase == GAME_PHASE_COUNTDOWN && !all_active_players_ready_locked(state)) {
        state->phase = GAME_PHASE_LOBBY;
        state->scheduled_start_ms = 0;
        returned_to_lobby = 1;

        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (state->players[i].isActive) {
                reset_player_to_lobby(&state->players[i], i);
            }
        }
    }

    pthread_mutex_unlock(&state->lock);
    return returned_to_lobby;
}

void game_manager_update_activity(GameState* state, int player_id) {
    pthread_mutex_lock(&state->lock);
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (state->players[i].isActive && state->players[i].playerId == player_id) {
            state->players[i].lastActivityTime = time(NULL);
            break;
        }
    }
    pthread_mutex_unlock(&state->lock);
}

int game_manager_mark_ready(GameState* state, int playerId) {
    int should_schedule = 0;
    int marked_ready = 0;

    pthread_mutex_lock(&state->lock);

    if (state->phase == GAME_PHASE_LOBBY) {
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (state->players[i].isActive && state->players[i].playerId == playerId) {
                state->players[i].isReady = 1;
                state->players[i].lastActivityTime = time(NULL);
                marked_ready = 1;
                break;
            }
        }

        if (marked_ready && all_active_players_ready_locked(state)) {
            schedule_game_start_locked(state);
            should_schedule = 1;
        }
    }

    pthread_mutex_unlock(&state->lock);
    return should_schedule;
}

void game_manager_reset_to_lobby(GameState* state) {
    pthread_mutex_lock(&state->lock);

    state->phase = GAME_PHASE_LOBBY;
    state->scheduled_start_ms = 0;

    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (state->players[i].isActive) {
            reset_player_to_lobby(&state->players[i], i);
        }
    }

    pthread_mutex_unlock(&state->lock);
}

void game_manager_broadcast(GameState* state, const void* packet, size_t size) {
    pthread_mutex_lock(&state->lock);
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (state->players[i].isActive) {
            sendto(state->listenfd_socket, packet, size, 0,
                   (struct sockaddr*)&state->players[i].client_addr,
                   sizeof(state->players[i].client_addr));
        }
    }
    pthread_mutex_unlock(&state->lock);
}

int game_manager_is_race_active(GameState* state) {
    int is_race_active;

    pthread_mutex_lock(&state->lock);
    is_race_active = (state->phase == GAME_PHASE_RACE);
    pthread_mutex_unlock(&state->lock);

    return is_race_active;
}

int game_manager_has_countdown_expired(GameState* state) {
    int should_start = 0;

    pthread_mutex_lock(&state->lock);

    if (state->phase == GAME_PHASE_COUNTDOWN && now_ms() >= state->scheduled_start_ms) {
        state->phase = GAME_PHASE_RACE;
        state->scheduled_start_ms = 0;
        should_start = 1;
    }

    pthread_mutex_unlock(&state->lock);
    return should_start;
}

uint32_t game_manager_get_remaining_countdown_ms(GameState* state) {
    uint32_t remaining_ms = 0;

    pthread_mutex_lock(&state->lock);

    if (state->phase == GAME_PHASE_COUNTDOWN && state->scheduled_start_ms > 0) {
        const uint64_t current_ms = now_ms();
        if (state->scheduled_start_ms > current_ms) {
            remaining_ms = (uint32_t)(state->scheduled_start_ms - current_ms);
        }
    }

    pthread_mutex_unlock(&state->lock);
    return remaining_ms;
}
