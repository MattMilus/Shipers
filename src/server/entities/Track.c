#include "Track.h"
#include <math.h>
#include <stdlib.h>

Buoy* track_buoys = NULL;
size_t track_buoy_count = 0;
static size_t track_capacity = 0;
FinishBuoy track_finish_buoy;

static void push_buoy(Vector2f pos) {
    if (track_buoy_count >= track_capacity) {
        track_capacity = (track_capacity == 0) ? 64 : track_capacity * 2;
        track_buoys = realloc(track_buoys, track_capacity * sizeof(Buoy));
    }
    track_buoys[track_buoy_count].position = pos;
    track_buoys[track_buoy_count].radius = 25.0f;
    track_buoy_count++;
}

static Vector2f get_bspline_point(Vector2f p0, Vector2f p1, Vector2f p2, Vector2f p3, float t) {
    float it = 1.0f - t;
    float t2 = t * t;
    float t3 = t2 * t;

    float b0 = (it * it * it) / 6.0f;
    float b1 = (3.0f * t3 - 6.0f * t2 + 4.0f) / 6.0f;
    float b2 = (-3.0f * t3 + 3.0f * t2 + 3.0f * t + 1.0f) / 6.0f;
    float b3 = t3 / 6.0f;

    Vector2f result;
    result.x = b0 * p0.x + b1 * p1.x + b2 * p2.x + b3 * p3.x;
    result.y = b0 * p0.y + b1 * p1.y + b2 * p2.y + b3 * p3.y;
    return result;
}

void track_generate(const Vector2f* control_points, size_t count) {
    if (count == 0) return;

    //track_buoy_count = 0; // delete for multiple bounds

    const float separation_distance = 50.0f;
    const int segments_per_curve = 20;

    size_t padded_count = count + 4;
    Vector2f* padded_points = malloc(padded_count * sizeof(Vector2f));

    padded_points[0] = control_points[0];
    padded_points[1] = control_points[0];
    for (size_t i = 0; i < count; ++i) {
        padded_points[i + 2] = control_points[i];
    }
    padded_points[padded_count - 2] = control_points[count - 1];
    padded_points[padded_count - 1] = control_points[count - 1];

    for (size_t i = 0; i < padded_count - 3; ++i) {
        for (int j = 0; j <= segments_per_curve; ++j) {
            if (j == 0 && i > 0) continue;

            float t = (float)j / (float)segments_per_curve;
            Vector2f next_pos = get_bspline_point(
                padded_points[i],
                padded_points[i + 1],
                padded_points[i + 2],
                padded_points[i + 3],
                t
            );

            if (track_buoy_count > 0) {
                Vector2f last_pos = track_buoys[track_buoy_count - 1].position;
                if (hypotf(next_pos.x - last_pos.x, next_pos.y - last_pos.y) < separation_distance) {
                    continue;
                }
            }

            push_buoy(next_pos);
        }
    }

    free(padded_points);
}

void track_set_finish(const Vector2f finish_buoy_pos) {
    track_finish_buoy.buoy.position = finish_buoy_pos;
    track_finish_buoy.buoy.radius = 25.0f;
    track_finish_buoy.finish_radius = DEFAULT_FINISH_RADIUS;
}

void track_cleanup(void) {
    free(track_buoys);
    track_buoys = NULL;
    track_buoy_count = 0;
    track_capacity = 0;
}

int isBoatFinished(const Vector2f boat_position, const FinishBuoy* finish_buoy) {
    if (finish_buoy == NULL) {
        return 0;
    }

    const float dx = boat_position.x - finish_buoy->buoy.position.x;
    const float dy = boat_position.y - finish_buoy->buoy.position.y;

    const float distance_squared = (dx * dx) + (dy * dy);

    const float radius_squared = finish_buoy->finish_radius * finish_buoy->finish_radius;

    return distance_squared <= radius_squared;
}