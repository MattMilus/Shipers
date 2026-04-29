//
// Created by Wiktor on 14.03.2026.
//

#include "ConnectionHandler.h"
#include "../ServerPackets.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>

void accept_connection(char* buffer, int sock, struct sockaddr_in *client_addr, GameState *gameState) {
    printf("Handling connection from client\n");
    int new_player_id = game_manager_add_player(gameState, client_addr);
    printf("New player id %d\n", new_player_id);

    if (new_player_id == -1) {
        // Wyślij do klienta pakiet z błędem np. MSG_SERVER_FULL
    } else {
        PacketAccepted response;
        response.type = MSG_ACCEPTED;
        response.player_id = new_player_id;

        sendto(sock, &response, sizeof(PacketAccepted), 0,
               (struct sockaddr*)client_addr, sizeof(struct sockaddr_in));

        printf("Sending accept to client\n");
        // @Todo: After adding lobby ensure proper communication and remove it from this place
        PacketGameStart startStatePacket;
        startStatePacket.type = MSG_GAME_START;
        startStatePacket.player_id = new_player_id;

        startStatePacket.active_players_count = 0;

        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (gameState->players[i].isActive) {
                PlayerSnapshot snapshot;
                snapshot.player_id = gameState->players[i].playerId;
                snapshot.x = gameState->players[i].boat.position.x;
                snapshot.y = gameState->players[i].boat.position.y;
                snapshot.currentAngle = gameState->players[i].boat.current_angle;
                snapshot.rotation = gameState->players[i].boat.rotation;
                snapshot.throttle = gameState->players[i].boat.throttle;

                startStatePacket.players[startStatePacket.active_players_count] = snapshot;
                startStatePacket.active_players_count++;
            }
        }

        if (startStatePacket.active_players_count > 0) {
            for (int i = 0; i < MAX_PLAYERS; i++) {
                if (gameState->players[i].isActive) {
                    sendto(gameState->listenfd_socket, &startStatePacket, sizeof(PacketGameStart), 0,
                           (struct sockaddr*)&gameState->players[i].client_addr, sizeof(struct sockaddr_in));
                }
            }
        }

    }
}

void player_disconnect(char* buffer, int sock, struct sockaddr_in *client_addr, GameState *gameState) {
    PacketDisconnect *disconnectPacket = (PacketDisconnect *)buffer;

    game_manager_remove_player(gameState, disconnectPacket->player_id);

    PacketPlayerDisconnected response;
    response.type = MSG_PLAYER_DISCONNECTED;
    response.player_id = disconnectPacket->player_id;

    if (gameState->current_player_count > 0) {
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (gameState->players[i].isActive) {
                sendto(sock, &response, sizeof(PacketPlayerDisconnected), 0,
                       (struct sockaddr*)&gameState->players[i].client_addr, sizeof(struct sockaddr_in));
            }
        }
    }
}

void* timeout_checker(void* arg) {
    GameState* state = (GameState*)arg;

    for (;;) {
        sleep(2);
        time_t now = time(NULL);

        pthread_mutex_lock(&state->lock);

        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (state->players[i].isActive) {
                if (now - state->players[i].lastActivityTime > TIMEOUT_SECONDS) {
                    fprintf(stderr, "[TIMEOUT] Player %d not responding. Releasing slot.\n", state->players[i].playerId);
                    state->players[i].isActive = false;
                    state->current_player_count--;

                    PacketPlayerDisconnected response;
                    response.type = MSG_PLAYER_DISCONNECTED;
                    response.player_id = state->players[i].playerId;

                    for (int j = 0; j < MAX_PLAYERS; j++) {
                        if (state->players[j].isActive) {
                            sendto(state->listenfd_socket, &response, sizeof(PacketPlayerDisconnected), 0,
                            (struct sockaddr*)&state->players[j].client_addr, sizeof(struct sockaddr_in));
                        }
                    }
                }
            }
        }

        pthread_mutex_unlock(&state->lock);
    }
    return NULL;
}