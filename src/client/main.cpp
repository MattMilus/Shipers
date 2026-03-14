#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
#include <cstring>
#include <map>

#include "managers/web_managers/ServerPackets.h"
#include "managers/GameManager.h"
#include "managers/Renderer.h"
#include "entities/Player.h"
#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/Packet.hpp"
#include "SFML/Network/UdpSocket.hpp"

using namespace sf;

Vector2f normalize(const Vector2f& source) {
    float length = sqrt((source.x * source.x) + (source.y * source.y));
    if (length != 0)
        return Vector2f(source.x / length, source.y / length);

    return source;
}

int main() {
    GameManager *gameManager = new GameManager();
    Renderer *renderer = new Renderer(gameManager);
    RenderWindow& window = *renderer->initialize();

    // @Todo: after connecting to server, server should create id for you
    if (gameManager->connectToServer() == -1) {
        std::cerr << "Error connecting to server." << std::endl;
        return -1;
    }

    // @Todo: Change after connecting to server to create remote player when server tells you about new player joining
    int remotePlayerId = 2;
    gameManager->addBoat(remotePlayerId, sf::Vector2f(200.f, 200.f));
    // @Todo: If you want to test 2 player movement simultaneously uncomment line below and comment one above
    // @Todo: Remember to get rid of this after connecting to server
    //gameManager->addPlayer(remotePlayerId, sf::Vector2f(200.f, 200.f));

    Clock globalClock;
    Clock deltaClock;

    while (window.isOpen()) {
        float time = globalClock.getElapsedTime().asSeconds();
        float deltaTime = deltaClock.restart().asSeconds();

        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        if (Player* localPlayer = gameManager->getPlayer()) {
            localPlayer->handleInput(deltaTime);
        }

        // @Todo: Test remote player input, change to real input after connecting to server
        /*if (Player* remotePlayer = gameManager->getPlayerById(remotePlayerId)) {
            remotePlayer->handleInput(-deltaTime);
        }*/

        for (auto& [id, boat] : gameManager->getActiveBoats()) {
            boat->update(deltaTime);

            // @Todo: Add collisions or something
        }

        gameManager->handleCollisions();

        //window.clear();
        renderer->render(time);
        //window.display();
    }

    return 0;
}