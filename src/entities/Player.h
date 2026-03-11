//
// Created by Wiktor on 11.03.2026.
//

#ifndef PLAYER_H
#define PLAYER_H


#pragma once
#include "Boat.h"
#include <SFML/Window/Keyboard.hpp>

class Player : public Boat {
public:
    Player(sf::Vector2f startPos);

    void handleInput(float deltaTime);
};



#endif //PLAYER_H
