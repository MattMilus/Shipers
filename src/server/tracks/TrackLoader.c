//
// Created by Wiktor on 27.05.2026.
//

#include "TrackLoader.h"
#include "tracks/Track1.h"

void TrackLoader_loadTrack(Tracks track) {
    switch (track) {
        case TRACK_1:
            Track1_loadTrack();
            break;

        // Niestety mamy tylko jedną trasę :(
        default:
            Track1_loadTrack();
            break;
    }
}