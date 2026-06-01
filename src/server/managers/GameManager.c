#include "GameManager.h"

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include "../ServerPackets.h"
#include "../entities/Track.h"
#include "../tracks/TrackLoader.h"

uint64_t now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ((uint64_t)ts.tv_sec * 1000ULL) + ((uint64_t)ts.tv_nsec / 1000000ULL);
}

static Vector2f race_spawn_for_slot(const int slot) {
    return track_spawn_points[slot];
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
    player->isFinished = 0;
    player->boat.finishedInfoSent = 0;

    player->boat.velocity = (Vector2f){0.0f, 0.0f};
    player->boat.rotation = 0.0f;
    player->boat.throttle = 0.0f;
}

static void move_player_to_race_start(Player* player, const int slot) {
    const Vector2f spawn = race_spawn_for_slot(slot);
    boat_init(&player->boat, spawn);
    player->boat.current_angle = RACE_START_ANGLE_DEGREES;
    player->boat.rotation = 0.0f;
    player->boat.throttle = 0.0f;
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

    state->player_finished_count = 0;
    state->winner_time = 0;
    state->race_start_ms = 0;

    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (state->players[i].isActive) {
            state->players[i].isFinished = 0;
            state->players[i].boat.finishedInfoSent = 0;

            move_player_to_race_start(&state->players[i], i);
        }
    }
}

void game_manager_init(GameState* state, const int listenfd_socket) {
    pthread_mutex_init(&state->lock, NULL);

    state->current_player_count = 0;
    state->listenfd_socket = listenfd_socket;
    state->phase = GAME_PHASE_LOBBY;
    state->scheduled_start_ms = 0;
    state->player_finished_count = 0;
    state->winner_time = 0;
    state->race_start_ms = 0;

    TrackLoader_loadTrack(TRACK_1);

    for (int i = 0; i < MAX_PLAYERS; i++) {
        state->players[i].isActive = 0;
        state->players[i].isFinished = 0;
        state->players[i].isReady = 0;
        state->players[i].playerId = i;
        write_player_nickname(state->players[i].nickname, sizeof(state->players[i].nickname), NULL, i);
        boat_init(&state->players[i].boat, race_spawn_for_slot(i));
        state->players[i].lastActivityTime = 0;
        memset(&state->players[i].client_addr, 0, sizeof(state->players[i].client_addr));
    }

    /* Initialize coins: 8 groups of 8 coins near origin for testing */
    state->coins_bits = 0; /* 0 -> none collected */
    const float groupSpacing = 16.0f;
    const float coinRadius = 8.0f;
    Vector2f offsets[8] = {
        { -12.f, -12.f }, { 0.f, -16.f }, { 12.f, -12.f }, { 16.f, 0.f },
        { 12.f, 12.f }, { 0.f, 16.f }, { -12.f, 12.f }, { -16.f, 0.f }
    };

    for (int g = 0; g < 8; ++g) {
        Vector2f basePos = { g * groupSpacing, 0.0f };
        for (int c = 0; c < 8; ++c) {
            int idx = g * 8 + c;
            coin_init(&state->coins[idx], (Vector2f){ basePos.x + offsets[c].x, basePos.y + offsets[c].y }, coinRadius, idx);
            state->coins[idx].cooldown_until_ms = 0;
            state->coins[idx].owner_player_id = -1;
        }
    }
}

int game_manager_add_player(GameState* state, struct sockaddr_in* client_addr, const char* nickname) {
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
            }
        }
    }

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
        state->race_start_ms = now_ms();
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

/* Collision between two players (have access to GameState to modify coins and broadcast) */
static void playerBoatCollision(GameState* state, Player* p1, Player* p2) {
    Boat* b1 = &p1->boat;
    Boat* b2 = &p2->boat;

    const float minDist = 2.0f * COLLIDER_RADIUS;
    const float dx = b2->position.x - b1->position.x;
    const float dy = b2->position.y - b1->position.y;
    const float distSq = (dx * dx) + (dy * dy);

    if (distSq < minDist * minDist && distSq > 0.0001f) {
        const float dist = sqrtf(distSq);
        const float nx = dx / dist;
        const float ny = dy / dist;

        const float overlap = minDist - dist;
        const float pushX = nx * (overlap * 0.5f);
        const float pushY = ny * (overlap * 0.5f);

        b1->position.x -= pushX;
        b1->position.y -= pushY;
        b2->position.x += pushX;
        b2->position.y += pushY;

        const float dvx = b2->velocity.x - b1->velocity.x;
        const float dvy = b2->velocity.y - b1->velocity.y;

        const float vn = (dvx * nx) + (dvy * ny);

        if (vn > 0.0f) return;

        const float e = 1.0f;
        const float impulse = -(1.0f + e) * vn * 0.5f;

        const float impulseX = nx * impulse;
        const float impulseY = ny * impulse;

        b1->velocity.x -= impulseX;
        b1->velocity.y -= impulseY;
        b2->velocity.x += impulseX;
        b2->velocity.y += impulseY;

        // Determine normal velocity components for each boat (approx using current velocities)
        float v1n = (b1->velocity.x * nx) + (b1->velocity.y * ny);
        float v2n = (b2->velocity.x * nx) + (b2->velocity.y * ny);

        float velocityDifference = fabsf(v1n - v2n);

        // Discrete drop rules
        const float MIN_DIFF_VELOCITY = 50.0f;   // below -> no drops
        const float MAX_DIFF_VELOCITY = 500.0f;  // above -> drop half of victim-owned
        const float STEP = 50.0f;                // 1 coin per STEP above MIN
        const uint32_t COOLDOWN_MS = 3000;       // respawn cooldown

        if (velocityDifference < MIN_DIFF_VELOCITY) {
            return; // no drops
        }

        // Identify attacker and victim by who has larger normal velocity
        Player* attacker = (v1n > v2n) ? p1 : p2;
        Player* victim = (v1n > v2n) ? p2 : p1;

        int coinsToDrop = 0;
        if (velocityDifference >= MAX_DIFF_VELOCITY) {
            // drop half of victim-owned coins
            int ownedCount = 0;
            for (int ci = 0; ci < 64; ++ci) {
                if (state->coins[ci].owner_player_id == victim->playerId && state->coins[ci].active == 0) {
                    ownedCount++;
                }
            }
            coinsToDrop = ownedCount / 2;
        } else {
            // discrete mapping: floor((diff - MIN)/STEP) + 1
            float effective = velocityDifference - MIN_DIFF_VELOCITY;
            coinsToDrop = (int)floorf(effective / STEP) + 1;
        }

        if (coinsToDrop <= 0) return;

        // Only drop victim-owned collected coins. If victim has none, no drops occur.
        int victimOwnedCount = 0;
        for (int ci = 0; ci < 64; ++ci) {
            if (state->coins[ci].owner_player_id == victim->playerId && state->coins[ci].active == 0) {
                victimOwnedCount++;
            }
        }

        if (victimOwnedCount <= 0) return; // victim has no collected coins -> nothing to drop

        if (coinsToDrop > victimOwnedCount) coinsToDrop = victimOwnedCount;

        uint64_t now = now_ms();
        int dropped = 0;

        // Respawn only victim-owned coins
        for (int ci = 0; ci < 64 && dropped < coinsToDrop; ++ci) {
            Coin* coin = &state->coins[ci];
            if (coin->owner_player_id == victim->playerId && coin->active == 0) {
                // respawn coin near collision
                float angle = ((float)dropped / (float)coinsToDrop) * 6.2831853f + 0.5f;
                float radius = 20.0f + (dropped * 6.0f);
                float cx = b1->position.x + cosf(angle) * radius;
                float cy = b1->position.y + sinf(angle) * radius;

                coin->position.x = cx;
                coin->position.y = cy;
                coin->active = 1;
                coin->owner_player_id = -1;
                coin->cooldown_until_ms = now + COOLDOWN_MS;

                // clear collected bit
                state->coins_bits &= ~(1ULL << (uint64_t)coin->index);

                // deduct points from victim
                victim->boat.points -= 100;
                if (victim->boat.points < 0) victim->boat.points = 0;

                // notify clients
                PacketCoinRespawn respkt;
                respkt.type = MSG_COIN_RESPAWN;
                respkt.coin_index = coin->index;
                respkt.x = coin->position.x;
                respkt.y = coin->position.y;
                respkt.cooldown_ms = COOLDOWN_MS;

                for (int pi = 0; pi < MAX_PLAYERS; ++pi) {
                    if (state->players[pi].isActive) {
                        sendto(state->listenfd_socket, &respkt, sizeof(respkt), 0,
                               (struct sockaddr*)&state->players[pi].client_addr,
                               sizeof(state->players[pi].client_addr));
                    }
                }

                dropped++;
            }
        }

        if (dropped > 0) {
            // broadcast updated coins bits
            PacketCoinsState pkt;
            pkt.type = MSG_COINS_STATE;
            pkt.coins_bits = state->coins_bits;
            for (int pi = 0; pi < MAX_PLAYERS; ++pi) {
                if (state->players[pi].isActive) {
                    sendto(state->listenfd_socket, &pkt, sizeof(pkt), 0,
                           (struct sockaddr*)&state->players[pi].client_addr,
                           sizeof(state->players[pi].client_addr));
                }
            }
            // Immediately broadcast updated player states so clients see points changes at once
            PacketGameState statePkt;
            statePkt.type = MSG_GAME_STATE;
            statePkt.active_players_count = 0;
            for (int i = 0; i < MAX_PLAYERS; i++) {
                if (state->players[i].isActive) {
                    PlayerSnapshot snap;
                    snap.player_id = state->players[i].playerId;
                    snap.x = state->players[i].boat.position.x;
                    snap.y = state->players[i].boat.position.y;
                    snap.currentAngle = state->players[i].boat.current_angle;
                    snap.rotation = state->players[i].boat.rotation;
                    snap.throttle = state->players[i].boat.throttle;
                    snap.velocityX = state->players[i].boat.velocity.x;
                    snap.velocityY = state->players[i].boat.velocity.y;
                    snap.points = state->players[i].boat.points;

                    statePkt.players[statePkt.active_players_count] = snap;
                    statePkt.active_players_count++;
                }
            }

            for (int pi = 0; pi < MAX_PLAYERS; ++pi) {
                if (state->players[pi].isActive) {
                    sendto(state->listenfd_socket, &statePkt, sizeof(statePkt), 0,
                           (struct sockaddr*)&state->players[pi].client_addr,
                           sizeof(state->players[pi].client_addr));
                }
            }
        }
    }
}

void buoyCollision(Boat* boat, Vector2f buoyPos, float buoyRadius) {
    const float minDist = COLLIDER_RADIUS + buoyRadius;
    const float minDistSq = minDist * minDist;

    const float dx = buoyPos.x - boat->position.x;
    const float dy = buoyPos.y - boat->position.y;
    const float distSq = (dx * dx) + (dy * dy);

    if (distSq < minDistSq && distSq > 0.0001f) {
        const float dist = sqrtf(distSq);
        const float nx = dx / dist;
        const float ny = dy / dist;

        const float overlap = minDist - dist;
        boat->position.x -= nx * overlap;
        boat->position.y -= ny * overlap;

        const float dvx = -boat->velocity.x;
        const float dvy = -boat->velocity.y;

        const float vn = (dvx * nx) + (dvy * ny);

        if (vn > 0.0f) return;

        const float e = 1.0f;
        const float impulse = -(1.0f + e) * vn;

        boat->velocity.x -= nx * impulse;
        boat->velocity.y -= ny * impulse;
    }
}

void game_manager_resolve_collisions(GameState* state) {
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (!state->players[i].isActive) continue;
        if (state->players[i].isFinished) continue;

        for (int j = i + 1; j < MAX_PLAYERS; j++) {
            if (!state->players[j].isActive) continue;
            if (state->players[j].isFinished) continue;

            playerBoatCollision(state, &state->players[i], &state->players[j]);
        }

        for (int j = 0; j < track_buoy_count; j++) {
            const Vector2f buoyPos = track_buoys[j].position;
            const float buoyRadius = track_buoys[j].radius;

            buoyCollision(&state->players[i].boat, buoyPos, buoyRadius);
        }

        /* Coin collisions */
        for (int ci = 0; ci < 64; ++ci) {
            Coin* coin = &state->coins[ci];
            if (!coin->active) continue;

            const float dx = coin->position.x - state->players[i].boat.position.x;
            const float dy = coin->position.y - state->players[i].boat.position.y;
            const float distSq = (dx * dx) + (dy * dy);
            const float minDist = COLLIDER_RADIUS + coin->radius;
            if (distSq < minDist * minDist && distSq > 0.0001f) {
                uint64_t now = now_ms();
                if (coin->cooldown_until_ms > now) {
                    // coin is on cooldown, ignore collection
                    continue;
                }

                /* collect coin */
                coin->active = 0;
                coin->owner_player_id = state->players[i].playerId; // set owner on collection
                state->coins_bits |= (1ULL << (uint64_t)coin->index);

                /* increase player's points */
                state->players[i].boat.points += 100; /* coin value */

                /* broadcast updated coins state to all players (we are already locked here) */
                PacketCoinsState pkt;
                pkt.type = MSG_COINS_STATE;
                pkt.coins_bits = state->coins_bits;
                for (int pi = 0; pi < MAX_PLAYERS; ++pi) {
                    if (state->players[pi].isActive) {
                        sendto(state->listenfd_socket, &pkt, sizeof(pkt), 0,
                               (struct sockaddr*)&state->players[pi].client_addr,
                               sizeof(state->players[pi].client_addr));
                    }
                }
            }
        }
    }
}

int get_finish_points(GameState* state, uint64_t finishing_time) {
    int points = WINNING_POINTS - POINTS_LOSS_FOR_SECOND * ((finishing_time - state->winner_time) / 1000.f);
    if (points < 0) points = 0;

    return points;
}