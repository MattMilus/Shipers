#include "Coin.h"

void coin_init(Coin* coin, Vector2f pos, float radius, int index) {
    coin->position = pos;
    coin->radius = radius;
    coin->active = 1;
    coin->index = index;
}
