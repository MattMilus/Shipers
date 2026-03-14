//
// Created by Wiktor on 14.03.2026.
//

#include "ConnectionManager.h"

#include <cstring>
#include <iostream>
#include <optional>
#include <string>

#include "ServerPackets.h"
#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/Socket.hpp"
#include "SFML/Network/UdpSocket.hpp"
#include "SFML/System/Clock.hpp"

#include "../../env.h"

int ConnectionManager::connectToServer() {
    std::string ipString;
    std::cout << "--- SHIPERS CLIENT ---\n";
    std::cout << "Enter server IP (eg. 127.0.0.1 for local game): ";
    std::cin >> ipString;

    std::optional<sf::IpAddress> serverIp = sf::IpAddress::resolve(ipString);
    if (!serverIp) {
        std::cerr << "Invalid IP address!\n";
        return -1;
    }

    sf::UdpSocket socket;

    PacketConnect connectPacket {MSG_CONNECT};

    if (socket.send(&connectPacket, sizeof(connectPacket), *serverIp, ENV_SERVER_PORT) != sf::Socket::Status::Done) {
        std::cerr << "Error while connecting to server!\n";
        return -1;
    }

    socket.setBlocking(false);
    sf::Clock timeoutClock;

    while (timeoutClock.getElapsedTime().asSeconds() < 5.0f) {
        char buffer[1024];
        std::size_t received;
        std::optional<sf::IpAddress> senderIp;
        unsigned short senderPort;

        if (socket.receive(buffer, sizeof(buffer), received, senderIp, senderPort) == sf::Socket::Status::Done) {
            MsgHeader header;
            std::memcpy(&header, buffer, sizeof(MsgHeader));

            if (header.type == MSG_ACCEPTED) {
                if (received == sizeof(PacketAccepted)) {
                    PacketAccepted acceptedPacket;
                    std::memcpy(&acceptedPacket, buffer, sizeof(PacketAccepted));

                    PacketJoin joinPacket{
                        MSG_JOIN,
                        acceptedPacket.player_id
                    };
                    if (socket.send(&joinPacket, sizeof(joinPacket), *serverIp, ENV_SERVER_PORT) != sf::Socket::Status::Done) {
                        std::cerr << "Error while connecting to server!\n";
                        return -1;
                    }

                    return acceptedPacket.player_id;
                }
            }
        }
    }

    std::cerr << "No response from the server (Timeout). Make sure the server is working.\n";
    return -1;
}
