#ifndef TRACK_H
#define TRACK_H

#include <stddef.h>
#include "Boat.h"

typedef struct {
    Vector2f position;
    float radius;
} Buoy;

extern Buoy* track_buoys;
extern size_t track_buoy_count;

void track_generate(const Vector2f* control_points, size_t count);
void track_cleanup(void);

#endif 