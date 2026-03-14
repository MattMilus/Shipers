#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
#include <map>

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
    std::string ipString;
    std::cout << "--- SHIPERS CLIENT ---\n";
    std::cout << "Podaj adres IP serwera (np. 127.0.0.1 dla gry lokalnej): ";
    std::cin >> ipString;

    // SFML 3.0 używa std::optional do rozwiązywania adresów IP
    std::optional<IpAddress> serverIp = IpAddress::resolve(ipString);
    if (!serverIp) {
        std::cerr << "Nieprawidlowy adres IP!\n";
        return 1;
    }

    // Ustawiamy port zgodny z Twoim nowym serwerem w C (5000)
    unsigned short serverPort = 5000;
    UdpSocket socket;

    std::cout << "Laczenie z serwerem " << serverIp->toString() << ":" << serverPort << "...\n";

    // Budujemy i wysyłamy pakiet powitalny
    std::string connectMsg = "CONNECT";
    // Wysyłamy surowe bajty stringa (bez null-terminatora)
    if (socket.send(connectMsg.c_str(), connectMsg.size(), *serverIp, serverPort) != Socket::Status::Done) {
        std::cerr << "Blad wysylania zadania polaczenia!\n";
        return 1;
    }

    int myPlayerId = -1;
    bool connected = false;
    socket.setBlocking(false);
    Clock timeoutClock;

    while (timeoutClock.getElapsedTime().asSeconds() < 5.0f) {
        char buffer[1024];
        std::size_t received;
        std::optional<IpAddress> senderIp;
        unsigned short senderPort;

        if (socket.receive(buffer, sizeof(buffer), received, senderIp, senderPort) == Socket::Status::Done) {
            buffer[received] = '\0'; // Zabezpieczenie końca stringa
            std::string response(buffer);

            // Sprawdzamy surowy tekst
            if (response.rfind("ACCEPTED", 0) == 0) { // Czy zaczyna się od ACCEPTED
                // Wyciągamy ID (wszystko po spacji)
                myPlayerId = std::stoi(response.substr(9));
                connected = true;
                std::cout << "Polaczono pomyslnie! Nadano ID gracza: " << myPlayerId << "\n";
                break;
            }
        }
    }

    if (!connected) {
        std::cerr << "Brak odpowiedzi od serwera (Timeout). Upewnij sie, ze serwer dziala.\n";
        return 1;
    }


    GameManager *gameManager = new GameManager();
    Renderer *renderer = new Renderer(gameManager);
    RenderWindow& window = *renderer->initialize();

    // @Todo: after connecting to server, server should create id for you
    gameManager->addPlayer(myPlayerId, sf::Vector2f(400.f, 300.f));

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

        if (Player* localPlayer = gameManager->getPlayerById(myPlayerId)) {
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