#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
#include <map>

#include "entities/Player.h"

using namespace sf;

Vector2f normalize(const Vector2f& source) {
    float length = sqrt((source.x * source.x) + (source.y * source.y));
    if (length != 0)
        return Vector2f(source.x / length, source.y / length);

    return source;
}

int main() {
    Glsl::Vec2 uPathHistory[1024];
    std::map<int, std::unique_ptr<Boat>> activeBoats;


    // @Todo: after connecting to server, server should create id for you
    int myPlayerId = 1;

    activeBoats[myPlayerId] = std::make_unique<Player>(sf::Vector2f(400.f, 300.f));

    // @Todo: Change after connecting to server
    int remotePlayerId = 2;
    activeBoats[remotePlayerId] = std::make_unique<Boat>(sf::Vector2f(200.f, 200.f));

    RenderWindow window(VideoMode({ 800, 600 }), "GPU Ripples");
    window.setFramerateLimit(60);

    Shader checkerShader;
    if (!checkerShader.loadFromFile("checker.frag", Shader::Type::Fragment)) {
        std::cerr << "Failed to load checker shader\n";
        return 1;
    }

    Shader waveShader;
    if (!waveShader.loadFromFile("wave.frag", Shader::Type::Fragment)) {
        std::cerr << "Failed to load wave shader\n";
        return 2;
    }
    Texture boatTexture("Sprite.png");
	Sprite boatSprite(boatTexture);
	boatSprite.setScale({ 0.5f, 0.5f });
	boatSprite.setOrigin({ boatTexture.getSize().x / 2.f, boatTexture.getSize().y / 2.f });

    RenderTexture renderTex(Vector2u{ 800, 600 });
    RectangleShape screenQuad(Vector2f{ 800.f, 600.f });

    Player player({ 400.f, 300.f });

    for (int i = 255; i >= 0; --i) {
        uPathHistory[i] = Glsl::Vec2(0.5f, 0.5f);
    }

    Clock globalClock;
    Clock deltaClock;

    checkerShader.setUniform("uResolution", Glsl::Vec2(800.f, 600.f));
    waveShader.setUniform("uResolution", Glsl::Vec2(800.f, 600.f));

    while (window.isOpen()) {
        float time = globalClock.getElapsedTime().asSeconds();
        float deltaTime = deltaClock.restart().asSeconds();

        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        auto it = activeBoats.find(myPlayerId);
        if (it != activeBoats.end()) {
            Player* localPlayer = dynamic_cast<Player*>(it->second.get());
            if (localPlayer) {
                localPlayer->handleInput(deltaTime);
            }
        }

        for (auto& [id, boat] : activeBoats) {
            boat->update(deltaTime);

            // @Todo: Add collisions or something
        }

        for (auto& [id, boat] : activeBoats) {
            boatSprite.setPosition(boat->getPosition());
            boatSprite.setRotation(sf::degrees(boat->getCurrentAngle()));
        }

        for (int i = 255; i > 0; --i) {
            uPathHistory[i] = uPathHistory[i - 1];
        }

        // Normalizing player pos to [0, 1] range for shader
        sf::Vector2f currentPos = player.getPosition();
        uPathHistory[0] = Glsl::Vec2(currentPos.x / 800.f, currentPos.y / 600.f);

        renderTex.clear();
        renderTex.draw(screenQuad, &checkerShader);
        renderTex.display();

        waveShader.setUniform("image", renderTex.getTexture());
        waveShader.setUniform("uTime", time);
        waveShader.setUniformArray("uPathHistory", uPathHistory, 256);

        window.clear();
        window.draw(screenQuad, &waveShader);

        for (auto& [id, boat] : activeBoats) {
            window.draw(boatSprite);
        }

        window.display();
    }

    return 0;
}