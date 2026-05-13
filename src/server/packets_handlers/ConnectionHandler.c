#include "ConnectionHandler.h"
#include "../ServerPackets.h"

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

static void notify_return_to_lobby(GameState* gameState) {
    PacketReturnToLobby response;
    response.type = MSG_RETURN_TO_LOBBY;
    game_manager_broadcast(gameState, &response, sizeof(response));
}

static void send_timeout_packet(int sock, const struct sockaddr_in* client_addr, const int player_id) {
    PacketTimeout timeoutPacket;
    timeoutPacket.type = MSG_TIMEOUT;
    timeoutPacket.player_id = player_id;

    sendto(sock, &timeoutPacket, sizeof(timeoutPacket), 0,
           (const struct sockaddr*)client_addr, sizeof(struct sockaddr_in));
}

static void write_lobby_nickname(char* destination, const size_t size, const char* nickname, const int player_id) {
    if (nickname != NULL && nickname[0] != '\0') {
        snprintf(destination, size, "%s", nickname);
        return;
    }

    snprintf(destination, size, "Player%d", player_id);
}

void accept_connection(char* buffer, const int sock, struct sockaddr_in *client_addr, GameState *gameState) {
    (void)buffer;

    const int new_player_id = game_manager_add_player(gameState, client_addr, NULL);
    if (new_player_id == -1) {
        fprintf(stderr, "Server full, rejecting player.\n");
        return;
    }

    PacketAccepted response;
    response.type = MSG_ACCEPTED;
    response.player_id = new_player_id;

    sendto(sock, &response, sizeof(response), 0,
           (struct sockaddr*)client_addr, sizeof(struct sockaddr_in));
}

void player_join_lobby(char* buffer, const int sock, struct sockaddr_in *client_addr, GameState *gameState) {
    PacketJoinLobby *joinPacket = (PacketJoinLobby *)buffer;
    PacketAckJoinLobby response;
    PacketNewPlayerJoin broadcastPacket;
    PacketGameStart startStatePacket;
    uint32_t remaining_countdown_ms = 0;
    int should_send_countdown = 0;
    int should_send_start_state = 0;
    int joined_player_found = 0;

    response.type = MSG_ACK_JOIN_LOBBY;
    response.player_id = joinPacket->player_id;
    response.active_players_count = 0;

    pthread_mutex_lock(&gameState->lock);

    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (!gameState->players[i].isActive) {
            continue;
        }

        if (gameState->players[i].playerId == joinPacket->player_id) {
            write_lobby_nickname(
                gameState->players[i].nickname,
                sizeof(gameState->players[i].nickname),
                joinPacket->nickname,
                joinPacket->player_id
            );
            gameState->players[i].lastActivityTime = time(NULL);
            joined_player_found = 1;
        }
    }

    if (joined_player_found) {
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (!gameState->players[i].isActive) {
                continue;
            }

            LobbyPlayerSnapshot snapshot;
            snapshot.player_id = gameState->players[i].playerId;
            snapshot.is_ready = gameState->players[i].isReady;
            snprintf(snapshot.nickname, sizeof(snapshot.nickname), "%s", gameState->players[i].nickname);
            response.players[response.active_players_count++] = snapshot;
        }

        broadcastPacket.type = MSG_NEW_PLAYER_JOIN;
        broadcastPacket.player_id = joinPacket->player_id;
        write_lobby_nickname(
            broadcastPacket.nickname,
            sizeof(broadcastPacket.nickname),
            joinPacket->nickname,
            joinPacket->player_id
        );

        if (gameState->phase == GAME_PHASE_COUNTDOWN) {
            remaining_countdown_ms = game_manager_get_remaining_countdown_ms(gameState);
            should_send_countdown = 1;
            should_send_start_state = 1;

            startStatePacket.type = MSG_GAME_START;
            startStatePacket.player_id = joinPacket->player_id;
            startStatePacket.active_players_count = 0;

            for (int i = 0; i < MAX_PLAYERS; i++) {
                if (!gameState->players[i].isActive) {
                    continue;
                }

                PlayerSnapshot snapshot;
                snapshot.player_id = gameState->players[i].playerId;
                snapshot.x = gameState->players[i].boat.position.x;
                snapshot.y = gameState->players[i].boat.position.y;
                snapshot.currentAngle = gameState->players[i].boat.current_angle;
                snapshot.rotation = gameState->players[i].boat.rotation;
                snapshot.throttle = gameState->players[i].boat.throttle;
                snapshot.velocityX = gameState->players[i].boat.velocity.x;
                snapshot.velocityY = gameState->players[i].boat.velocity.y;

                startStatePacket.players[startStatePacket.active_players_count++] = snapshot;
            }
        }
    }

    pthread_mutex_unlock(&gameState->lock);

    if (!joined_player_found) {
        fprintf(stderr, "Rejecting lobby join for unknown player id %d\n", joinPacket->player_id);
        return;
    }

    sendto(sock, &response, sizeof(response), 0,
           (struct sockaddr*)client_addr, sizeof(struct sockaddr_in));

    game_manager_broadcast(gameState, &broadcastPacket, sizeof(broadcastPacket));

    if (should_send_countdown) {
        PacketGameScheduledStart scheduledPacket;
        scheduledPacket.type = MSG_GAME_SCHEDULED_START;
        scheduledPacket.countdown_ms = remaining_countdown_ms;

        sendto(sock, &scheduledPacket, sizeof(scheduledPacket), 0,
               (struct sockaddr*)client_addr, sizeof(struct sockaddr_in));
    }

    if (should_send_start_state && startStatePacket.active_players_count > 0) {
        sendto(sock, &startStatePacket, sizeof(startStatePacket), 0,
               (struct sockaddr*)client_addr, sizeof(struct sockaddr_in));
    }
}

void player_disconnect(char* buffer, const int sock, struct sockaddr_in *client_addr, GameState *gameState) {
    (void)sock;
    (void)client_addr;
    PacketDisconnect *disconnectPacket = (PacketDisconnect *)buffer;

    const int remove_result = game_manager_remove_player(gameState, disconnectPacket->player_id);
    if (remove_result < 0) {
        return;
    }

    PacketPlayerDisconnected response;
    response.type = MSG_PLAYER_DISCONNECTED;
    response.player_id = disconnectPacket->player_id;

    game_manager_broadcast(gameState, &response, sizeof(response));
    if (remove_result == 1) {
        notify_return_to_lobby(gameState);
    }
}

void* timeout_checker(void* arg) {
    GameState* state = (GameState*)arg;

    for (;;) {
        sleep(2);
        const time_t now = time(NULL);
        int timed_out_ids[MAX_PLAYERS];
        struct sockaddr_in timed_out_addrs[MAX_PLAYERS];
        int timed_out_count = 0;

        pthread_mutex_lock(&state->lock);

        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (!state->players[i].isActive) {
                continue;
            }

            if (now - state->players[i].lastActivityTime > TIMEOUT_SECONDS) {
                timed_out_ids[timed_out_count] = state->players[i].playerId;
                timed_out_addrs[timed_out_count] = state->players[i].client_addr;
                timed_out_count++;
            }
        }

        pthread_mutex_unlock(&state->lock);

        int should_notify_lobby = 0;

        for (int i = 0; i < timed_out_count; i++) {
            send_timeout_packet(state->listenfd_socket, &timed_out_addrs[i], timed_out_ids[i]);

            const int remove_result = game_manager_remove_player(state, timed_out_ids[i]);
            if (remove_result < 0) {
                continue;
            }

            PacketPlayerDisconnected response;
            response.type = MSG_PLAYER_DISCONNECTED;
            response.player_id = timed_out_ids[i];
            game_manager_broadcast(state, &response, sizeof(response));

            if (remove_result == 1) {
                should_notify_lobby = 1;
            }
        }

        if (should_notify_lobby) {
            notify_return_to_lobby(state);
        }
    }

    return NULL;
}
