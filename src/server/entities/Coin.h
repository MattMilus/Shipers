#ifndef COIN_H
#define COIN_H

#include "Boat.h"
#include <stdint.h>

typedef struct {
    Vector2f position;
    float radius;
    int active; // 1 active (available), 0 collected
    int index; // 0..63
    uint64_t cooldown_until_ms; // timestamp in ms until which coin cannot be collected
    int owner_player_id; // -1 if unowned, otherwise player id who collected it
} Coin;

void coin_init(Coin* coin, Vector2f pos, float radius, int index);

#endif // COIN_H
