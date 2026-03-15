#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
#include <cstring>
#include <map>

#include "managers/GameManager.h"
#include "managers/Renderer.h"
#include "entities/Player.h"
#include "managers/web_managers/PacketHandler.h"
#include "managers/web_managers/StateManager.h"
#include "SFML/Network/IpAddress.hpp"
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

    if (gameManager->connectToServer() == -1) {
        std::cerr << "Error connecting to server." << std::endl;
        return -1;
    }

    Clock globalClock;
    Clock deltaClock;
    Clock networkClock;
    const float NETWORK_TICK_RATE = 1.0f / 30.0f;

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

        char buffer[2048];
        std::size_t received;
        std::optional<IpAddress> senderIp;
        unsigned short senderPort;

        while (gameManager->getUdpSocket()->receive(buffer, sizeof(buffer), received, senderIp, senderPort) == sf::Socket::Status::Done) {
            PacketHandler::handleIncomingPacket(buffer, received, gameManager);
        }

        for (auto& [id, boat] : gameManager->getActiveBoats()) {
            boat->update(deltaTime);
        }

        if (networkClock.getElapsedTime().asSeconds() >= NETWORK_TICK_RATE) {
            StateManager::sendMoveInformation(gameManager);
            networkClock.restart();
        }

        renderer->render(time);
    }

    return 0;
}