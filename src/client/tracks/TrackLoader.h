//
// Created by Wiktor on 27.05.2026.
//

#ifndef SHIPERS_TRACKLOADER_H
#define SHIPERS_TRACKLOADER_H

#include "../managers/GameManager.h"

enum Tracks {
    Track1
};

class TrackLoader {
public:
    static void loadTrack(Tracks track, GameManager* gameManager);
};


#endif //SHIPERS_TRACKLOADER_H
