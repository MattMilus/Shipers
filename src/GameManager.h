//
// Created by Wiktor on 12.03.2026.
//

#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H
#include <map>
#include <memory>

#include "entities/Boat.h"
#include "entities/Player.h"


class GameManager {
private:
    std::map<int, std::unique_ptr<Boat>> activeBoats;
public:
    GameManager();

    // Returns the player ID (index) of the newly added player, or -1 if max players reached
    int addPlayer(int id, sf::Vector2f startPos);
    int addBoat(int id, sf::Vector2f startPos);
    [[nodiscard]] const std::map<int, std::unique_ptr<Boat>>& getActiveBoats() const;
    [[nodiscard]] Boat* getBoatById(int id) const;
    [[nodiscard]] Player* getPlayerById(int id) const;
};



#endif //GAMEMANAGER_H
