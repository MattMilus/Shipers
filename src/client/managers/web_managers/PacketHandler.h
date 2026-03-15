//
// Created by Wiktor on 14.03.2026.
//

#ifndef SHIPERS_PACKETHANDLER_H
#define SHIPERS_PACKETHANDLER_H

#include <cstddef>

#include "../GameManager.h"


class PacketHandler {
public:
    static void handleIncomingPacket(char* buffer, std::size_t receivedSize, GameManager* gameManager);
};


#endif //SHIPERS_PACKETHANDLER_H