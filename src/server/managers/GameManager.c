<<<<<<< HEAD
#include "GameManager.h"

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>

static uint64_t now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ((uint64_t)ts.tv_sec * 1000ULL) + ((uint64_t)ts.tv_nsec / 1000000ULL);
}

static Vector2f race_spawn_for_slot(const int slot) {
    Vector2f spawn = {300.0f + ((float)slot * 90.0f), 450.0f};
    return spawn;
}

static void write_player_nickname(char* destination, const size_t destination_size, const char* nickname, const int player_id) {
    if (nickname != NULL && nickname[0] != '\0') {
        snprintf(destination, destination_size, "%s", nickname);
        return;
    }

    snprintf(destination, destination_size, "Player%d", player_id);
}

static void reset_player_to_lobby(Player* player) {
    player->isReady = 0;
    player->boat.velocity = (Vector2f){0.0f, 0.0f};
    player->boat.throttle = 0.0f;
}

static void move_player_to_race_start(Player* player, const int slot) {
    const Vector2f spawn = race_spawn_for_slot(slot);
    boat_init(&player->boat, spawn);
    player->boat.current_angle = RACE_START_ANGLE_DEGREES;
    player->boat.angle_command = RACE_START_ANGLE_DEGREES;
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

void game_manager_init(GameState* state, const int listenfd_socket) {
=======
//
// Created by Wiktor on 14.03.2026.
//

#include <math.h>

#include "GameManager.h"

#include <stdio.h>

#include "../packets_handlers/StateManager.h"

void game_manager_init(GameState* state, int listenfd_socket) {
>>>>>>> fe0a395227799a729b4d41892a4ca4981f9b3b93
    pthread_mutex_init(&state->lock, NULL);

    state->current_player_count = 0;
    state->listenfd_socket = listenfd_socket;
    state->phase = GAME_PHASE_LOBBY;
    state->scheduled_start_ms = 0;

    for (int i = 0; i < MAX_PLAYERS; i++) {
        state->players[i].isActive = 0;
        state->players[i].isReady = 0;
        state->players[i].playerId = i;
        write_player_nickname(state->players[i].nickname, sizeof(state->players[i].nickname), NULL, i);
        boat_init(&state->players[i].boat, race_spawn_for_slot(i));
    }

    state->players[3].isActive = true;
    boat_init(&state->players[3].boat, (Vector2f){100.0f, 100.0f});
    state->players[3].lastActivityTime = time(NULL) + 20;
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
            boat_init(&state->players[i].boat, race_spawn_for_slot(i));
            write_player_nickname(state->players[i].nickname, sizeof(state->players[i].nickname), nickname, state->players[i].playerId);

            pthread_mutex_unlock(&state->lock);
            return state->players[i].playerId;
        }
    }

    pthread_mutex_unlock(&state->lock);
    return -1;
}

int game_manager_remove_player(GameState* state, const int playerId) {
    int returned_to_lobby = 0;
    int removed_player = 0;

    pthread_mutex_lock(&state->lock);

    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (state->players[i].playerId == playerId && state->players[i].isActive) {
            state->players[i].isActive = 0;
            state->current_player_count--;
            reset_player_to_lobby(&state->players[i]);
            write_player_nickname(state->players[i].nickname, sizeof(state->players[i].nickname), NULL, state->players[i].playerId);
            removed_player = 1;
            break;
        }
    }

    if (!removed_player) {
        pthread_mutex_unlock(&state->lock);
        return -1;
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
                reset_player_to_lobby(&state->players[i]);
            }
        }
    }

    pthread_mutex_unlock(&state->lock);
    return returned_to_lobby;
}

void game_manager_update_activity(GameState* state, const int player_id) {
    pthread_mutex_lock(&state->lock);
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (state->players[i].isActive && state->players[i].playerId == player_id) {
            state->players[i].lastActivityTime = time(NULL);
            break;
        }
    }
    pthread_mutex_unlock(&state->lock);
}

<<<<<<< HEAD
int game_manager_mark_ready(GameState* state, const int playerId) {
    int marked_ready = 0;

    pthread_mutex_lock(&state->lock);

    if (state->phase == GAME_PHASE_LOBBY) {
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (state->players[i].isActive && state->players[i].playerId == playerId && !state->players[i].isReady) {
                state->players[i].isReady = 1;
                state->players[i].lastActivityTime = time(NULL);
                marked_ready = 1;
                break;
=======
void game_manager_resolve_collisions(GameState* state) {
    float minDist = 2.0f * COLLIDER_RADIUS;
    float minDistSq = minDist * minDist;

    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (!state->players[i].isActive) continue;

        for (int j = i + 1; j < MAX_PLAYERS; j++) {
            if (!state->players[j].isActive) continue;

            Boat* b1 = &state->players[i].boat;
            Boat* b2 = &state->players[j].boat;

            float dx = b2->position.x - b1->position.x;
            float dy = b2->position.y - b1->position.y;

            float distSq = (dx * dx) + (dy * dy);

            if (distSq < minDistSq && distSq > 0.0001f) {
                float dist = sqrtf(distSq);

                float overlap = minDist - dist;

                float nx = dx / dist;
                float ny = dy / dist;

                float pushX = nx * (overlap * 0.5f);
                float pushY = ny * (overlap * 0.5f);

                b1->position.x -= pushX;
                b1->position.y -= pushY;

                b2->position.x += pushX;
                b2->position.y += pushY;

                float bounceForce = 20.0f;
                b1->velocity.x -= nx * bounceForce;
                b1->velocity.y -= ny * bounceForce;
                b2->velocity.x += nx * bounceForce;
                b2->velocity.y += ny * bounceForce;
>>>>>>> fe0a395227799a729b4d41892a4ca4981f9b3b93
            }
        }
    }

<<<<<<< HEAD
    pthread_mutex_unlock(&state->lock);
    return marked_ready;
}

int game_manager_try_schedule_start(GameState* state) {
    int should_schedule = 0;

    pthread_mutex_lock(&state->lock);

    if (state->phase == GAME_PHASE_LOBBY && all_active_players_ready_locked(state)) {
        schedule_game_start_locked(state);
        should_schedule = 1;
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
            reset_player_to_lobby(&state->players[i]);
        }
    }

    pthread_mutex_unlock(&state->lock);
}

void game_manager_broadcast(GameState* state, const void* packet, const size_t size) {
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
=======
    printf("B%d: vel x: %f, vel y: %f\n", 0, state->players[0].boat.velocity.x, state->players[0].boat.velocity.y);
}
>>>>>>> fe0a395227799a729b4d41892a4ca4981f9b3b93
