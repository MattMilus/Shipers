//
// Created by Wiktor on 14.03.2026.
//

#include "ConnectionHandler.h"

#include <stdio.h>
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
    }
}