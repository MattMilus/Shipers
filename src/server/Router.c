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
        case MSG_JOIN_LOBBY: {
            fprintf(stderr, "Received message JOIN_LOBBY\n");
            player_join_lobby(buffer, sock, client_addr, gameState);
            break;
        }
        case MSG_MOVE: {
            //fprintf(stderr, "Move player\n");
            movePlayer(buffer, sock, client_addr, gameState);
            break;
        }
        case MSG_PLAYER_READY: {
            fprintf(stderr, "Received message PLAYER_READY\n");
            player_ready(buffer, sock, client_addr, gameState);
            break;
        }
        case MSG_DISCONNECT: {
            fprintf(stderr, "Received message DISCONNECT\n");
            player_disconnect(buffer, sock, client_addr, gameState);
            break;
        }
        case MSG_I_AM_ALIVE: {
            fprintf(stderr, "Received message I_AM_ALIVE\n");
            player_heartbeat(buffer, sock, client_addr, gameState);
            break;
        }


        default:
            fprintf(stderr, "Received unkown message type: %d\n", header->type);
            break;
    }
}
