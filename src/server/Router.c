//
// Created by Wiktor on 14.03.2026.
//

// router.c
#include <stdio.h>
#include "Router.h"

#include "managers/GameManager.h"
#include "packets_handlers/ConnectionHandler.h"
#include "packets_handlers/StateManager.h"

void route_message(char *buffer, int read_size, int sock, struct sockaddr_in *client_addr, GameState* gameState) {
    if (read_size < sizeof(MsgHeader)) {
        fprintf(stderr, "Rejecting packet, too short (%d bytes).\n", read_size);
        return;
    }

    MsgHeader *header = (MsgHeader *)buffer;

    switch (header->type) {
        case MSG_CONNECT: {
            fprintf(stderr, "Received message CONNECT\n");
            accept_connection(buffer, sock, client_addr, gameState);
            break;
        }
        case MSG_MOVE: {
            //fprintf(stderr, "Move player\n");
            movePlayer(buffer, sock, client_addr, gameState);
            break;
        }


        default:
            fprintf(stderr, "Received unkown message type: %d\n", header->type);
            break;
    }
}