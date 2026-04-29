//
// Created by Wiktor on 14.03.2026.
//

#include <math.h>

#include "GameManager.h"

#include <stdio.h>

#include "../packets_handlers/StateManager.h"

void game_manager_init(GameState* state, int listenfd_socket) {
    pthread_mutex_init(&state->lock, NULL);

    state->current_player_count = 0;
    state->listenfd_socket = listenfd_socket;

    for (int i = 0; i < MAX_PLAYERS; i++) {
        state->players[i].isActive = false;
        state->players[i].playerId = i + 1;
    }

    state->players[3].isActive = true;
    boat_init(&state->players[3].boat, (Vector2f){100.0f, 100.0f});
    state->players[3].lastActivityTime = time(NULL) + 20;
}

int game_manager_add_player(GameState* state, struct sockaddr_in *client_addr) {
    pthread_mutex_lock(&state->lock);

    if (state->current_player_count >= MAX_PLAYERS) {
        pthread_mutex_unlock(&state->lock);
        return -1;
    }

    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (!state->players[i].isActive) {

            state->players[i].isActive = true;
            state->current_player_count++;

            state->players[i].client_addr = *client_addr;

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
            }
        }
    }

    printf("B%d: vel x: %f, vel y: %f\n", 0, state->players[0].boat.velocity.x, state->players[0].boat.velocity.y);
}