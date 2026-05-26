#ifndef TRACK_H
#define TRACK_H

#include <stddef.h>
#include "Boat.h"

#define DEFAULT_FINISH_RADIUS 100.0f

typedef struct {
    Vector2f position;
    float radius;
} Buoy;

typedef struct {
    Buoy buoy;
    float finish_radius;
} FinishBuoy;

extern Buoy* track_buoys;
extern size_t track_buoy_count;
extern FinishBuoy track_finish_buoy;

void track_generate(const Vector2f* control_points, size_t count, const Vector2f finish_buoy_pos);
void track_cleanup(void);
int isBoatFinished(const Vector2f boat_position, const FinishBuoy* finish_buoy);

#endif 