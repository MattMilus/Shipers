//
// Created by Wiktor on 27.05.2026.
//

#include "TrackLoader.h"

#include "tracks/Track1.h"

void TrackLoader::loadTrack(Tracks track, GameManager* gameManager) {
    switch (track) {
        case Track1: {
            Track1::loadTrack(gameManager);
            break;
        }

        // Sadly we have only one course :(
        default: {
            Track1::loadTrack(gameManager);
            break;
        }

    }
}
