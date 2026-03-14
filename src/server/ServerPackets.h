//
// Created by Wiktor on 14.03.2026.
//

#ifndef SHIPERS_MESSAGETYPEENUM_H
#define SHIPERS_MESSAGETYPEENUM_H

#include <stdint.h>
#include <netinet/in.h>

// Using uint32_t,to ensure, 4 bytes used on every system
typedef enum uint32_t {
    MSG_CONNECT = 1,
    MSG_ACCEPTED = 2,
    MSG_MOVE = 3
} MessageType;

typedef struct {
    MessageType type;
} MsgHeader;

struct PacketConnect {
    MsgHeader header;
};

typedef struct {
    MessageType type;
    int player_id;
} PacketAccepted;

typedef struct {
    MessageType type; // Zawsze MSG_MOVE
    int player_id;
    float x;
    float y;
} PacketMove;

#endif //SHIPERS_MESSAGETYPEENUM_H