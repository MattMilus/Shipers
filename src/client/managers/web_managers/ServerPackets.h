//
// Created by Wiktor on 14.03.2026.
//

#ifndef SHIPERS_SERVERPACKETS_H
#define SHIPERS_SERVERPACKETS_H

#include <cstdint>

#pragma pack(push, 1)

enum MessageType : std::uint32_t {
    MSG_CONNECT = 1,
    MSG_ACCEPTED = 2,
    MSG_JOIN = 3,
    MSG_MOVE = 10
};

struct MsgHeader {
    MessageType type;
};

struct PacketConnect {
    MessageType type;
};

struct PacketAccepted {
    MessageType type;
    int player_id;
};

struct PacketJoin {
    MessageType type;
    int player_id;
};

struct PacketMove {
    MessageType type;
    int player_id;
    float x;
    float y;
};

#pragma pack(pop)
#endif //SHIPERS_SERVERPACKETS_H