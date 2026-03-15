//
// Created by Wiktor on 14.03.2026.
//

#ifndef SHIPERS_ROUTER_H
#define SHIPERS_ROUTER_H

#include <netinet/in.h>
#include "ServerPackets.h"
#include "managers/GameManager.h"


void route_message(char *buffer, int read_size, int sock, struct sockaddr_in *client_addr, GameState* gameState);

#endif //SHIPERS_ROUTER_H