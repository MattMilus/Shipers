//
// Created by Wiktor on 14.03.2026.
//

#include "Boat.h"
#include <math.h>

void boat_init(Boat* boat, Vector2f start_pos) {
    boat->position = start_pos;
    boat->velocity = (Vector2f){0.0f, 0.0f};
    boat->current_angle = 0.0f;
    boat->angle_command = 0.0f;
    boat->throttle = 0.0f;

    boat->acceleration = 100.0f;
    boat->turn_speed = 90.0f;
    boat->drag_forward = 0.98f;
    boat->drag_lateral = 0.80f;
}

void boat_set_throttle(Boat* boat, float new_throttle) {
    boat->throttle = new_throttle;
}