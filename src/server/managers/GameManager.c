//
// Created by Wiktor on 14.03.2026.
//

#include "GameManager.h"

void game_manager_init(GameState* state, int listenfd_socket) {
    pthread_mutex_init(&state->lock, NULL);

    state->current_player_count = 0;
    state->listenfd_socket = listenfd_socket;

    for (int i = 0; i < MAX_PLAYERS; i++) {
        state->players[i].isActive = false;
        state->players[i].playerId = i + 1;
    }
}

int game_manager_add_player(GameState* state) {
    pthread_mutex_lock(&state->lock);

    if (state->current_player_count >= MAX_PLAYERS) {
        pthread_mutex_unlock(&state->lock);
        return -1;
    }

    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (!state->players[i].isActive) {

            state->players[i].isActive = true;
            state->current_player_count++;

            Vector2f start_pos;
            start_pos.x = 100.0f;
            start_pos.y = 100.0f + (i * 80.0f);

            boat_init(&state->players[i].boat, start_pos);

            int assigned_id = state->players[i].playerId;

            state->players[i].lastActivityTime = time(NULL);

            pthread_mutex_unlock(&state->lock);

            return assigned_id;
        }
    }

    pthread_mutex_unlock(&state->lock);
    return -1;
}

void game_manager_remove_player(GameState* state, int playerId) {
    pthread_mutex_lock(&state->lock);

    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (state->players[i].playerId == playerId && state->players[i].isActive) {
            state->players[i].isActive = false;
            state->current_player_count--;
            break;
        }
    }

    pthread_mutex_unlock(&state->lock);
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