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
    MSG_DISCONNECT = 3,
    MSG_PLAYER_DISCONNECTED = 4,
    MSG_TIMEOUT = 5,
    MSG_GAME_STATE = 100,
    MSG_MOVE = 101
} MessageType;

typedef struct {
    MessageType type;
} MsgHeader;

typedef struct {
    MsgHeader header;
} PacketConnect;

typedef struct {
    MessageType type;
    int player_id;
} PacketAccepted;

typedef struct {
    MessageType type;
    int player_id;
} PacketDisconnect;

typedef struct {
    MessageType type;
    int player_id;
} PacketPlayerDisconnected;

typedef struct {
    MessageType type;
    int player_id;
} PacketTimeout;

typedef struct {
    uint32_t type;
    int player_id;
    float x;
    float y;
    float currentAngle;
    float angleCommand;
    float throttle;
} PacketMove;

typedef struct {
    int player_id;
    float x;
    float y;
    float currentAngle;
    float angleCommand;
    float throttle;
} PlayerSnapshot;

typedef struct {
    uint32_t type;
    int active_players_count;
    PlayerSnapshot players[4];
} PacketGameState;

#endif //SHIPERS_MESSAGETYPEENUM_H