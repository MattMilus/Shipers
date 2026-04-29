#include <SFML/Graphics.hpp>
#include <cstdio>
#include <iostream>
#include <optional>
#include <string>

#include "entities/Player.h"
#include "managers/GameManager.h"
#include "managers/Panel.h"
#include "managers/Renderer.h"
#include "managers/TextField.h"
#include "managers/web_managers/PacketHandler.h"
#include "managers/web_managers/StateManager.h"

namespace {
constexpr float NETWORK_TICK_RATE = 1.0f / 30.0f;

std::string statusTextFor(const GameManager& gameManager, const std::string& disconnectedMessage) {
    if (!gameManager.isConnectedToServer()) {
        return disconnectedMessage;
    }

    switch (gameManager.getSessionPhase()) {
        case SessionPhase::Lobby:
            return gameManager.isReady() ? "Ready. Waiting for the remaining players."
                                         : "Connected. Click READY when everyone is here.";
        case SessionPhase::Countdown:
            return "All players are ready. Race starts soon.";
        case SessionPhase::Race:
            return "Race in progress.";
    }

    return disconnectedMessage;
}

void updateActionButton(Panel& button, const GameManager& gameManager) {
    if (!gameManager.isConnectedToServer()) {
        button.setStyle(sf::Color::White, 22, sf::Color(34, 112, 73), sf::Color(201, 255, 220), 2.0f);
        button.setHeldStyle(sf::Color(34, 112, 73), 22, sf::Color::White, sf::Color(34, 112, 73), 2.0f);
        button.setText("CONNECT");
        button.setHeldText("CONNECT");
        return;
    }

    if (gameManager.getSessionPhase() == SessionPhase::Lobby && !gameManager.isReady()) {
        button.setStyle(sf::Color::White, 22, sf::Color(38, 92, 196), sf::Color(219, 230, 255), 2.0f);
        button.setHeldStyle(sf::Color(38, 92, 196), 22, sf::Color::White, sf::Color(38, 92, 196), 2.0f);
        button.setText("READY");
        button.setHeldText("READY");
        return;
    }

    button.setStyle(sf::Color(235, 240, 248), 22, sf::Color(89, 106, 125), sf::Color(220, 228, 239), 2.0f);
    button.setHeldStyle(sf::Color(235, 240, 248), 22, sf::Color(89, 106, 125), sf::Color(220, 228, 239), 2.0f);
    if (gameManager.getSessionPhase() == SessionPhase::Countdown) {
        button.setText("STARTING");
        button.setHeldText("STARTING");
    } else {
        button.setText("WAITING");
        button.setHeldText("WAITING");
    }
}
}

int main() {
    printf("\033[2J\033[1;1H"); // 'Clear' console

    auto* gameManager = new GameManager();
    auto* renderer = new Renderer(gameManager);
    sf::RenderWindow& window = *renderer->initialize();

    auto regCursor = sf::Cursor::createFromSystem(sf::Cursor::Type::Arrow);
    auto handCursor = sf::Cursor::createFromSystem(sf::Cursor::Type::Hand);

    sf::Font uiFont("assets/fonts/arial.ttf");
    sf::Clock globalClock;
    sf::Clock deltaClock;
    sf::Clock networkClock;

    TextField serverIpField;
    serverIpField.setLabel("Server IP");
    serverIpField.setPlaceholder("127.0.0.1");
    serverIpField.setValue("127.0.0.1");
    serverIpField.setPosition({48.0f, 88.0f});
    serverIpField.setSize({300.0f, 50.0f});

    TextField nicknameField;
    nicknameField.setLabel("Nickname");
    nicknameField.setPlaceholder("Player0");
    nicknameField.setPosition({380.0f, 88.0f});
    nicknameField.setSize({220.0f, 50.0f});

    Panel actionButton;
    actionButton.setPosition({628.0f, 88.0f});
    actionButton.setSize({124.0f, 50.0f});
    updateActionButton(actionButton, *gameManager);

    std::string disconnectedMessage = "Enter server IP and nickname, then connect to the lobby.";
    bool wasConnectedLastFrame = false;

    actionButton.setButton([&]() {
        if (!gameManager->isConnectedToServer()) {
            const std::string nickname = nicknameField.getValue();
            if (gameManager->connectToServer(serverIpField.getValue(), nickname) == -1) {
                disconnectedMessage = "Could not connect to the server. Check the IP and try again.";
                return;
            }

            disconnectedMessage = "Connected to the lobby.";
            return;
        }

        if (gameManager->getSessionPhase() == SessionPhase::Lobby && !gameManager->isReady()) {
            StateManager::sendReady(gameManager);
        }
    });

    renderer->setOverlayDrawer([&](sf::RenderWindow& renderWindow) {
        if (gameManager->isConnectedToServer() && gameManager->getSessionPhase() == SessionPhase::Race) {
            return;
        }

        const sf::Vector2f windowSize(
            static_cast<float>(renderWindow.getSize().x),
            static_cast<float>(renderWindow.getSize().y)
        );

        sf::RectangleShape overlay(windowSize);
        overlay.setFillColor(sf::Color(9, 18, 30, 214));
        renderWindow.draw(overlay);

        sf::RectangleShape titleBand({windowSize.x, 54.0f});
        titleBand.setFillColor(sf::Color(20, 44, 70, 235));
        renderWindow.draw(titleBand);

        sf::Text title(uiFont);
        title.setCharacterSize(28);
        title.setFillColor(sf::Color::White);
        title.setString("Lobby");
        title.setPosition({48.0f, 10.0f});
        renderWindow.draw(title);

        sf::Text status(uiFont);
        status.setCharacterSize(18);
        status.setFillColor(sf::Color(220, 228, 239));
        status.setString(statusTextFor(*gameManager, disconnectedMessage));
        status.setPosition({48.0f, 154.0f});
        renderWindow.draw(status);

        if (gameManager->isConnectedToServer() && gameManager->getSessionPhase() == SessionPhase::Countdown) {
            char countdownBuffer[32];
            std::snprintf(countdownBuffer, sizeof(countdownBuffer), "Start in %.1fs", gameManager->getCountdownSecondsLeft());

            sf::Text countdown(uiFont);
            countdown.setCharacterSize(18);
            countdown.setFillColor(sf::Color(255, 228, 168));
            countdown.setString(countdownBuffer);
            countdown.setPosition({600.0f, 154.0f});
            renderWindow.draw(countdown);
        }

        serverIpField.draw(renderWindow);
        nicknameField.draw(renderWindow);
        actionButton.draw(renderWindow);

        const float tableLeft = 48.0f;
        const float tableTop = 212.0f;
        const float tableWidth = windowSize.x - (tableLeft * 2.0f);
        const float headerHeight = 48.0f;
        const float rowHeight = 62.0f;

        sf::RectangleShape header({tableWidth, headerHeight});
        header.setPosition({tableLeft, tableTop});
        header.setFillColor(sf::Color(30, 63, 97, 245));
        header.setOutlineColor(sf::Color(214, 227, 247));
        header.setOutlineThickness(2.0f);
        renderWindow.draw(header);

        sf::Text headerText(uiFont);
        headerText.setCharacterSize(18);
        headerText.setFillColor(sf::Color::White);
        headerText.setString("ID");
        headerText.setPosition({tableLeft + 24.0f, tableTop + 11.0f});
        renderWindow.draw(headerText);

        headerText.setString("Nickname");
        headerText.setPosition({tableLeft + 120.0f, tableTop + 11.0f});
        renderWindow.draw(headerText);

        headerText.setString("Status");
        headerText.setPosition({tableLeft + tableWidth - 180.0f, tableTop + 11.0f});
        renderWindow.draw(headerText);

        float currentRowTop = tableTop + headerHeight + 12.0f;
        int renderedRows = 0;

        for (const auto& [id, lobbyPlayer] : gameManager->getLobbyPlayers()) {
            sf::RectangleShape row({tableWidth, rowHeight});
            row.setPosition({tableLeft, currentRowTop});
            row.setFillColor(renderedRows % 2 == 0 ? sf::Color(241, 246, 252, 232)
                                                   : sf::Color(226, 236, 248, 232));
            row.setOutlineColor(sf::Color(191, 206, 226));
            row.setOutlineThickness(1.0f);
            renderWindow.draw(row);

            sf::Text cell(uiFont);
            cell.setCharacterSize(20);
            cell.setFillColor(sf::Color(20, 35, 55));
            cell.setString(std::to_string(id));
            cell.setPosition({tableLeft + 24.0f, currentRowTop + 14.0f});
            renderWindow.draw(cell);

            std::string displayedNickname = lobbyPlayer.nickname;
            if (id == gameManager->getPlayerId()) {
                displayedNickname += " (you)";
            }

            cell.setString(displayedNickname);
            cell.setPosition({tableLeft + 120.0f, currentRowTop + 14.0f});
            renderWindow.draw(cell);

            const bool playerReady =
                gameManager->getSessionPhase() == SessionPhase::Countdown || gameManager->isPlayerReady(id);

            cell.setFillColor(playerReady ? sf::Color(34, 112, 73) : sf::Color(145, 92, 28));
            cell.setString(playerReady ? "READY" : "WAITING");
            if (gameManager->getSessionPhase() == SessionPhase::Countdown) {
                cell.setString("COUNTDOWN");
            }
            cell.setPosition({tableLeft + tableWidth - 180.0f, currentRowTop + 14.0f});
            renderWindow.draw(cell);

            currentRowTop += rowHeight + 10.0f;
            renderedRows++;
        }

        if (renderedRows == 0) {
            sf::RectangleShape emptyState({tableWidth, rowHeight});
            emptyState.setPosition({tableLeft, currentRowTop});
            emptyState.setFillColor(sf::Color(241, 246, 252, 232));
            emptyState.setOutlineColor(sf::Color(191, 206, 226));
            emptyState.setOutlineThickness(1.0f);
            renderWindow.draw(emptyState);

            sf::Text emptyText(uiFont);
            emptyText.setCharacterSize(20);
            emptyText.setFillColor(sf::Color(58, 76, 102));
            emptyText.setString("No players in the lobby yet.");
            emptyText.setPosition({tableLeft + 24.0f, currentRowTop + 14.0f});
            renderWindow.draw(emptyText);
        }
    });

    while (window.isOpen()) {
        const float time = globalClock.getElapsedTime().asSeconds();
        const float deltaTime = deltaClock.restart().asSeconds();

        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            const bool showLobbyOverlay =
                !gameManager->isConnectedToServer() || gameManager->getSessionPhase() != SessionPhase::Race;
            const sf::Vector2f mousePos = {
                static_cast<float>(sf::Mouse::getPosition(window).x),
                static_cast<float>(sf::Mouse::getPosition(window).y)
            };

            if (const auto* textEntered = event->getIf<sf::Event::TextEntered>()) {
                if (showLobbyOverlay && !gameManager->isConnectedToServer()) {
                    if (serverIpField.isFocused()) {
                        serverIpField.handleTextEntered(textEntered->unicode);
                    } else if (nicknameField.isFocused()) {
                        nicknameField.handleTextEntered(textEntered->unicode);
                    }
                }
            }

            if (showLobbyOverlay && event->is<sf::Event::MouseButtonPressed>()) {
                if (!gameManager->isConnectedToServer() && serverIpField.contains(mousePos)) {
                    serverIpField.setFocused(true);
                    nicknameField.setFocused(false);
                } else if (!gameManager->isConnectedToServer() && nicknameField.contains(mousePos)) {
                    serverIpField.setFocused(false);
                    nicknameField.setFocused(true);
                } else {
                    serverIpField.setFocused(false);
                    nicknameField.setFocused(false);

                    if (actionButton.contains(mousePos)) {
                        actionButton.onClick();
                    }
                }
            }
        }

        const bool showLobbyOverlay =
            !gameManager->isConnectedToServer() || gameManager->getSessionPhase() != SessionPhase::Race;
        const sf::Vector2f mousePos = {
            static_cast<float>(sf::Mouse::getPosition(window).x),
            static_cast<float>(sf::Mouse::getPosition(window).y)
        };
        if (showLobbyOverlay && actionButton.contains(mousePos)) {
            window.setMouseCursor(*handCursor);
        } else {
            window.setMouseCursor(*regCursor);
        }

        if (sf::UdpSocket* socket = gameManager->getUdpSocket()) {
            char buffer[2048];
            std::size_t received = 0;
            std::optional<sf::IpAddress> senderIp;
            unsigned short senderPort = 0;

            while (socket->receive(buffer, sizeof(buffer), received, senderIp, senderPort) == sf::Socket::Status::Done) {
                PacketHandler::handleIncomingPacket(buffer, received, gameManager);

                if (gameManager->getUdpSocket() != socket || !gameManager->isConnectedToServer()) {
                    break;
                }
            }
        }

        if (wasConnectedLastFrame && !gameManager->isConnectedToServer()) {
            disconnectedMessage = "Connection to the server was lost.";
        }
        wasConnectedLastFrame = gameManager->isConnectedToServer();

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

        if (gameManager->getSessionPhase() == SessionPhase::Race) {
            gameManager->handleCollisions();
        }

        if (networkClock.getElapsedTime().asSeconds() >= NETWORK_TICK_RATE && gameManager->shouldSendMoves()) {
            StateManager::sendMoveInformation(gameManager);
            networkClock.restart();
        }

        updateActionButton(actionButton, *gameManager);
        renderer->render(time);
    }

    printf("\033[2J\033[1;1H"); // 'Clear' console
    gameManager->disconnectFromServer();

    return 0;
}
