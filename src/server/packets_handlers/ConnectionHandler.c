//
// Created by Wiktor on 14.03.2026.
//

#include "ConnectionHandler.h"

#include <stdio.h>
#include <sys/socket.h>

void accept_connection(char* buffer, int sock, struct sockaddr_in *client_addr) {
    PacketAccepted response;
    response.type = MSG_ACCEPTED;
    response.player_id = 1;

    sendto(sock, &response, sizeof(PacketAccepted), 0,
           (struct sockaddr*)client_addr, sizeof(struct sockaddr_in));
}
