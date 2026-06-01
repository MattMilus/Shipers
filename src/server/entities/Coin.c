#include "Coin.h"
#include "../managers/GameManager.h"

void coin_init(Coin* coin, Vector2f pos, float radius, int index) {
    coin->position = pos;
    coin->radius = radius;
    coin->active = 1;
    coin->index = index;
    coin->cooldown_until_ms = 0;
    coin->owner_player_id = -1;
}
