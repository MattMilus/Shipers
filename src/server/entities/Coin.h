#ifndef COIN_H
#define COIN_H

#include "Boat.h"

typedef struct {
    Vector2f position;
    float radius;
    int active; // 1 active (available), 0 collected
    int index; // 0..63
} Coin;

void coin_init(Coin* coin, Vector2f pos, float radius, int index);

#endif // COIN_H
