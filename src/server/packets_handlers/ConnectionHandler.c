#include "ConnectionHandler.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>

static void notify_return_to_lobby(GameState* gameState) {
    PacketReturnToLobby response;
    response.type = MSG_RETURN_TO_LOBBY;
    game_manager_broadcast(gameState, &response, sizeof(response));
}

void accept_connection(char* buffer, int sock, struct sockaddr_in *client_addr, GameState *gameState) {
    PacketConnect *connectPacket = (PacketConnect *)buffer;

    printf("Handling connection from client '%s'\n", connectPacket->nickname);
    int new_player_id = game_manager_add_player(gameState, client_addr, connectPacket->nickname);

    if (new_player_id == -1) {
        fprintf(stderr, "Server full, rejecting player '%s'\n", connectPacket->nickname);
        return;
    }

    PacketAccepted response;
    response.type = MSG_ACCEPTED;
    response.player_id = new_player_id;
    snprintf(response.nickname, sizeof(response.nickname), "%s", connectPacket->nickname);

    sendto(sock, &response, sizeof(response), 0,
           (struct sockaddr*)client_addr, sizeof(struct sockaddr_in));

    const uint32_t remaining_countdown_ms = game_manager_get_remaining_countdown_ms(gameState);

    pthread_mutex_lock(&gameState->lock);

    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (!gameState->players[i].isActive) {
            continue;
        }

        PacketNewPlayerJoin joinPacket;
        joinPacket.type = MSG_NEW_PLAYER_JOIN;
        joinPacket.player_id = gameState->players[i].playerId;
        snprintf(joinPacket.nickname, sizeof(joinPacket.nickname), "%s", gameState->players[i].nickname);

        if (gameState->players[i].playerId == new_player_id) {
            continue;
        }

        sendto(sock, &joinPacket, sizeof(joinPacket), 0,
               (struct sockaddr*)client_addr, sizeof(struct sockaddr_in));

        if (gameState->players[i].isReady) {
            PacketAckReady readyPacket;
            readyPacket.type = MSG_ACK_READY;
            readyPacket.player_id = gameState->players[i].playerId;

            sendto(sock, &readyPacket, sizeof(readyPacket), 0,
                   (struct sockaddr*)client_addr, sizeof(struct sockaddr_in));
        }
    }

    PacketNewPlayerJoin broadcastPacket;
    broadcastPacket.type = MSG_NEW_PLAYER_JOIN;
    broadcastPacket.player_id = new_player_id;
    snprintf(broadcastPacket.nickname, sizeof(broadcastPacket.nickname), "%s", connectPacket->nickname);

    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (!gameState->players[i].isActive || gameState->players[i].playerId == new_player_id) {
            continue;
        }

        sendto(sock, &broadcastPacket, sizeof(broadcastPacket), 0,
               (struct sockaddr*)&gameState->players[i].client_addr, sizeof(struct sockaddr_in));
    }

    if (gameState->phase == GAME_PHASE_COUNTDOWN) {
        PacketGameScheduledStart scheduledPacket;
        scheduledPacket.type = MSG_GAME_SCHEDULED_START;
        scheduledPacket.countdown_ms = remaining_countdown_ms;

        sendto(sock, &scheduledPacket, sizeof(scheduledPacket), 0,
               (struct sockaddr*)client_addr, sizeof(struct sockaddr_in));
    }

    pthread_mutex_unlock(&gameState->lock);
}

void player_disconnect(char* buffer, int sock, struct sockaddr_in *client_addr, GameState *gameState) {
    (void)client_addr;
    PacketDisconnect *disconnectPacket = (PacketDisconnect *)buffer;

    const int returned_to_lobby = game_manager_remove_player(gameState, disconnectPacket->player_id);

    PacketPlayerDisconnected response;
    response.type = MSG_PLAYER_DISCONNECTED;
    response.player_id = disconnectPacket->player_id;

    game_manager_broadcast(gameState, &response, sizeof(response));
    if (returned_to_lobby) {
        notify_return_to_lobby(gameState);
    }
}

void* timeout_checker(void* arg) {
    GameState* state = (GameState*)arg;

    for (;;) {
        sleep(2);
        time_t now = time(NULL);
        int timed_out_ids[MAX_PLAYERS];
        int timed_out_count = 0;

        pthread_mutex_lock(&state->lock);

        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (!state->players[i].isActive) {
                continue;
            }

            if (now - state->players[i].lastActivityTime > TIMEOUT_SECONDS) {
                timed_out_ids[timed_out_count++] = state->players[i].playerId;
                state->players[i].isActive = 0;
                state->current_player_count--;
                state->players[i].isReady = 0;
                state->players[i].nickname[0] = '\0';
            }
        }

        if (timed_out_count > 0) {
            state->phase = GAME_PHASE_LOBBY;
            state->scheduled_start_ms = 0;

            for (int i = 0; i < MAX_PLAYERS; i++) {
                if (state->players[i].isActive) {
                    Vector2f spawn = {100.0f, 100.0f + (i * 80.0f)};
                    boat_init(&state->players[i].boat, spawn);
                    state->players[i].isReady = 0;
                }
            }
        }

        pthread_mutex_unlock(&state->lock);

        for (int i = 0; i < timed_out_count; i++) {
            PacketPlayerDisconnected response;
            response.type = MSG_PLAYER_DISCONNECTED;
            response.player_id = timed_out_ids[i];
            game_manager_broadcast(state, &response, sizeof(response));
        }

        if (timed_out_count > 0) {
            notify_return_to_lobby(state);
        }
    }

    return NULL;
}
