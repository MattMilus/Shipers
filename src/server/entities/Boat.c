//
// Created by Wiktor on 14.03.2026.
//

#include "Boat.h"
#include <math.h>

#define PI 3.14159265f

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

void boat_update_physics(Boat* boat, float deltaTime) {
    if (fabsf(boat->angle_command) > 0.1f) {
        float rotationStep = boat->turn_speed * deltaTime;
        if (fabsf(boat->angle_command) > rotationStep) {
            if (boat->angle_command > 0.f) {
                boat->current_angle += rotationStep;
                boat->angle_command -= rotationStep;
            } else {
                boat->current_angle -= rotationStep;
                boat->angle_command += rotationStep;
            }
        }
    }

    while (boat->current_angle >= 360.f) boat->current_angle -= 360.f;
    while (boat->current_angle < 0.f) boat->current_angle += 360.f;

    float rad = (boat->current_angle - 90.f) * (PI / 180.f);

    Vector2f forwardVec = { cosf(rad), sinf(rad) };
    Vector2f rightVec = { -sinf(rad), cosf(rad) };

    float forwardVelocity = (boat->velocity.x * forwardVec.x) + (boat->velocity.y * forwardVec.y);
    float lateralVelocity = (boat->velocity.x * rightVec.x) + (boat->velocity.y * rightVec.y);

    forwardVelocity *= powf(boat->drag_forward, deltaTime * 60.f);
    lateralVelocity *= powf(boat->drag_lateral, deltaTime * 60.f);

    forwardVelocity += boat->throttle * boat->acceleration * deltaTime;

    boat->velocity.x = (forwardVec.x * forwardVelocity) + (rightVec.x * lateralVelocity);
    boat->velocity.y = (forwardVec.y * forwardVelocity) + (rightVec.y * lateralVelocity);

    boat->position.x += boat->velocity.x * deltaTime;
    boat->position.y += boat->velocity.y * deltaTime;
}