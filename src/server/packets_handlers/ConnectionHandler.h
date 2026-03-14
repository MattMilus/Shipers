//
// Created by Wiktor on 14.03.2026.
//

#ifndef SHIPERS_CONNECTHANDLER_H
#define SHIPERS_CONNECTHANDLER_H

#include "../ServerPackets.h"

void accept_connection(char* buffer, int sock, struct sockaddr_in *client_addr);
void player_join(char* buffer, int sock, struct sockaddr_in *client_addr);


#endif //SHIPERS_CONNECTHANDLER_H