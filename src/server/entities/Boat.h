//
// Created by Wiktor on 14.03.2026.
//

#ifndef SHIPERS_BOAT_H
#define SHIPERS_BOAT_H

#include <stdbool.h>

#define ACCELERATION 300.f
#define TURN_SPEED 120.f
#define DRAG_FORWARD 0.995f
#define DRAG_LATERAL 0.98f

#define COLLIDER_RADIUS 20.0f

typedef struct {
    float x;
    float y;
} Vector2f;

typedef struct {
    Vector2f position;
    Vector2f velocity;

    float current_angle;
    float rotation;

    float throttle;
    float acceleration;
    float turn_speed;

    float drag_forward;
    float drag_lateral;
} Boat;

void boat_init(Boat* boat, Vector2f start_pos);
void boat_update_physics(Boat* boat, float deltaTime);

#endif //SHIPERS_BOAT_H