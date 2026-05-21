#ifndef SHIPERS_MESSAGETYPEENUM_H
#define SHIPERS_MESSAGETYPEENUM_H

#include <stdint.h>

#pragma pack(push, 1)

typedef uint32_t MessageType;

enum {
    MSG_CONNECT = 1,
    MSG_ACCEPTED = 2,
    MSG_DISCONNECT = 3,
    MSG_PLAYER_DISCONNECTED = 4,
    MSG_TIMEOUT = 5,
    MSG_JOIN_LOBBY = 6,
    MSG_ACK_JOIN_LOBBY = 7,
    MSG_NEW_PLAYER_JOIN = 10,
    MSG_PLAYER_READY = 11,
    MSG_ACK_READY = 12,
    MSG_GAME_SCHEDULED_START = 13,
    MSG_RETURN_TO_LOBBY = 14,
    MSG_GAME_START = 100,
    MSG_GAME_STATE = 101,
    MSG_MOVE = 102,

    MSG_I_AM_ALIVE = 999
};

typedef struct {
    MessageType type;
} MsgHeader;

typedef struct {
    MessageType type;
} PacketConnect;

typedef struct {
    MessageType type;
    int player_id;
} PacketAccepted;

typedef struct {
    int player_id;
    int is_ready;
    char nickname[32];
} LobbyPlayerSnapshot;

typedef struct {
    MessageType type;
    int player_id;
    char nickname[32];
} PacketJoinLobby;

typedef struct {
    MessageType type;
    int player_id;
    int active_players_count;
    LobbyPlayerSnapshot players[4];
} PacketAckJoinLobby;

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
    MessageType type;
    int player_id;
    char nickname[32];
} PacketNewPlayerJoin;

typedef struct {
    MessageType type;
    int player_id;
} PacketPlayerReady;

typedef struct {
    MessageType type;
    int player_id;
} PacketAckReady;

typedef struct {
    MessageType type;
    uint32_t countdown_ms;
} PacketGameScheduledStart;

typedef struct {
    MessageType type;
} PacketReturnToLobby;

typedef struct {
    MessageType type;
    int player_id;
    float x;
    float y;
    float currentAngle;
    float rotation;
    float throttle;
} PacketMove;

typedef struct {
    int player_id;
    float x;
    float y;
    float currentAngle;
    float rotation;
    float throttle;
    float velocityX;
    float velocityY;
} PlayerSnapshot;

typedef struct {
    MessageType type;
    int player_id;
    int active_players_count;
    PlayerSnapshot players[4];
} PacketGameStart;

typedef struct {
    MessageType type;
    int active_players_count;
    PlayerSnapshot players[4];
} PacketGameState;

typedef struct {
    MessageType type;
    int player_id;
} PacketIAmAlive;

#pragma pack(pop)

#endif // SHIPERS_MESSAGETYPEENUM_H
