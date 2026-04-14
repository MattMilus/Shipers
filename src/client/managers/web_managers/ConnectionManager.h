//
// Created by Wiktor on 14.03.2026.
//

#ifndef SHIPERS_CONNECTIONMANAGER_H
#define SHIPERS_CONNECTIONMANAGER_H

#include "../GameManager.h"

class ConnectionManager {
public:
    static int connectToServer(GameManager* game_manager);
    static int disconnectFromServer(GameManager* game_manager);
};


#endif //SHIPERS_CONNECTIONMANAGER_H