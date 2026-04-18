#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>
#include <optional>

#include "entities/Player.h"
#include "managers/GameManager.h"
#include "managers/Renderer.h"
#include "managers/web_managers/PacketHandler.h"
#include "managers/web_managers/StateManager.h"

int main() {
    printf("\033[2J\033[1;1H");

    auto* gameManager = new GameManager();
    auto* renderer = new Renderer(gameManager);
    sf::RenderWindow& window = *renderer->initialize();

    if (gameManager->connectToServer() == -1) {
        std::cerr << "Error connecting to server." << std::endl;
        return -1;
    }

    auto regCursor = sf::Cursor::createFromSystem(sf::Cursor::Type::Arrow);
    auto handCursor = sf::Cursor::createFromSystem(sf::Cursor::Type::Hand);

    sf::Clock globalClock;
    sf::Clock deltaClock;
    sf::Clock networkClock;
    const float networkTickRate = 1.0f / 30.0f;

    size_t statusPanelId = renderer->addPanel();
    Panel& statusPanel = renderer->getPanel(statusPanelId);
    statusPanel.setPosition({10.f, 10.f});
    statusPanel.setSize({320.f, 90.f});
    statusPanel.setStyle(sf::Color::White, 20, sf::Color(20, 20, 20, 190), sf::Color::White, 2.0f);
    statusPanel.setText("Lobby");

    size_t readyPanelId = renderer->addPanel();
    Panel& readyPanel = renderer->getPanel(readyPanelId);
    readyPanel.setPosition({10.f, 120.f});
    readyPanel.setSize({220.f, 50.f});
    readyPanel.setStyle(sf::Color::Blue, 20, sf::Color::White, sf::Color::Blue, 2.0f);
    readyPanel.setHeldStyle(sf::Color::White, 20, sf::Color::Blue, sf::Color::White, 2.0f);
    readyPanel.setText("READY");
    readyPanel.setHeldText("READY");
    readyPanel.setButton([gameManager]() {
        if (gameManager->getSessionPhase() == SessionPhase::Lobby && !gameManager->isReady()) {
            StateManager::sendReady(gameManager);
        }
    });
    readyPanel.setHover([&window, &handCursor]() {
        window.setMouseCursor(*handCursor);
    });

    while (window.isOpen()) {
        const float time = globalClock.getElapsedTime().asSeconds();
        const float deltaTime = deltaClock.restart().asSeconds();

        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            const sf::Vector2f mousePos = {
                static_cast<float>(sf::Mouse::getPosition(window).x),
                static_cast<float>(sf::Mouse::getPosition(window).y)
            };
            const size_t pId = renderer->getPanelIdAt(mousePos);

            if (pId != static_cast<size_t>(-1)) {
                Panel& panel = renderer->getPanel(pId);
                panel.onHover();
                if (event->is<sf::Event::MouseButtonPressed>()) {
                    panel.onClick();
                } else if (event->is<sf::Event::MouseButtonReleased>()) {
                    renderer->releaseAllButtons();
                }
            } else {
                window.setMouseCursor(*regCursor);
            }
        }

        char buffer[2048];
        std::size_t received;
        std::optional<sf::IpAddress> senderIp;
        unsigned short senderPort;

        while (gameManager->getUdpSocket()->receive(buffer, sizeof(buffer), received, senderIp, senderPort) ==
               sf::Socket::Status::Done) {
            PacketHandler::handleIncomingPacket(buffer, received, gameManager);
        }

        gameManager->updateSessionState();

        if (gameManager->getSessionPhase() == SessionPhase::Race) {
            if (Player* localPlayer = gameManager->getPlayer()) {
                localPlayer->handleInput(deltaTime);
            }
        }

        for (auto& [id, boat] : gameManager->getActiveBoats()) {
            if (id == gameManager->getPlayerId()) {
                if (gameManager->getSessionPhase() == SessionPhase::Race) {
                    boat->updateLocal(deltaTime);
                }
            } else {
                boat->updateRemote(deltaTime);
            }
        }

        if (networkClock.getElapsedTime().asSeconds() >= networkTickRate && gameManager->shouldSendMoves()) {
            StateManager::sendMoveInformation(gameManager);
            networkClock.restart();
        }

        switch (gameManager->getSessionPhase()) {
            case SessionPhase::Lobby:
                if (gameManager->isReady()) {
                    statusPanel.setText("Lobby\nReady - waiting for others");
                    readyPanel.setText("WAITING...");
                } else {
                    statusPanel.setText("Lobby\nClick READY to join race");
                    readyPanel.setText("READY");
                }
                break;
            case SessionPhase::Countdown:
                statusPanel.setText("Race starts in %.1fs", gameManager->getCountdownSecondsLeft());
                readyPanel.setText("STARTING...");
                break;
            case SessionPhase::Race:
                statusPanel.setText("Race in progress");
                readyPanel.setText("IN RACE");
                break;
        }

        renderer->render(time);
    }

    printf("\033[2J\033[1;1H");
    gameManager->disconnectFromServer();

    return 0;
}
