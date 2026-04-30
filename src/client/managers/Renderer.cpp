//
// Created by Wiktor on 12.03.2026.
//

#include "Renderer.h"

#include <iostream>
#include <SFML/Graphics/CircleShape.hpp>
#include "Terminal.h"



Renderer::Renderer(GameManager* game_manager) 
    : gameManager(game_manager), 
      boatSprite(boatTexture),
      resolution(800.f, 600.f) {
}

size_t Renderer::getPanelIdAt(const sf::Vector2f& pos) {
    for (size_t i = 0; i < panels.size(); ++i) {
        if (panels[i].contains(pos)) return i;
    }
    return -1;
}

void Renderer::releaseAllButtons() {
	for (Panel& panel : panels) {
		if (panel.changeStyle) panel.switchStyle();
	}
}

void Renderer::loadShaders() {
    if (!checkerShader.loadFromFile("assets/shaders/checker.frag", sf::Shader::Type::Fragment)) {
        std::cerr << "Failed to load checker shader\n";
    }

    if (!waveShader.loadFromFile("assets/shaders/wave.frag", sf::Shader::Type::Fragment)) {
        std::cerr << "Failed to load wave shader\n";
    }

    checkerShader.setUniform("uResolution", resolution);
    waveShader.setUniform("uResolution", resolution);
}

void Renderer::loadTextures() {
    if (!boatTexture.loadFromFile("assets/textures/Sprite.png")) {
        std::cerr << "Failed to load boat texture\n";
    }
}

void Renderer::loadSprites() {
    boatSprite.setTexture(boatTexture, true);
    boatSprite.setScale({ 0.5f, 0.5f });
    boatSprite.setOrigin({ boatTexture.getSize().x / 2.f, boatTexture.getSize().y / 2.f });
}

sf::RenderWindow *Renderer::initialize() {
    window.create(sf::VideoMode({ 800, 600 }), "GPU Ripples");
    window.setFramerateLimit(60);

    loadShaders();
    loadTextures();
    loadSprites();

    if (!renderTex.resize(sf::Vector2u{ 800, 600 })) {
        std::cerr << "Failed to create/resize render texture\n";
    }
    screenQuad.setSize(sf::Vector2f{ 800.f, 600.f });

    for (int i = 255; i >= 0; --i) {
        uPathHistory[i] = sf::Glsl::Vec2(0.5f, 0.5f);
    }

    return &window;
}

void Renderer::renderBackground(float time) {
    // Shader array filling - shifting old positions and adding current one at the start
    int boatIndex = 0;
    for (auto& [id, boat] : gameManager->getActiveBoats()) {
        boatSprite.setPosition(boat->getPosition());
        boatSprite.setRotation(sf::degrees(boat->getCurrentAngle()));

        for (int i = BOAT_U_PATH_HISTORY_SIZE - 1; i > 0; --i) {
            uPathHistory[boatIndex*BOAT_U_PATH_HISTORY_SIZE + i] = uPathHistory[boatIndex*BOAT_U_PATH_HISTORY_SIZE + i - 1];
        }

        sf::Vector2f currentPos = boat->getPosition();
        // Normalizing player pos to [0, 1] range for shader
        uPathHistory[boatIndex*BOAT_U_PATH_HISTORY_SIZE] = sf::Glsl::Vec2(currentPos.x / 800.f, currentPos.y / 600.f);

        boatIndex++;
    }

    renderTex.clear();
    renderTex.draw(screenQuad, &checkerShader);
    renderTex.display();

    waveShader.setUniform("image", renderTex.getTexture());
    waveShader.setUniform("uTime", time);
    waveShader.setUniformArray("uPathHistory", uPathHistory, TOTAL_HISTORY_SIZE);
    waveShader.setUniform("uPlayerId", gameManager->getPlayerId());

    window.clear();
    window.draw(screenQuad, &waveShader);
}

void Renderer::renderBoats() {
    if (gameManager->getSessionPhase() != SessionPhase::Race) {
        return;
    }

    Player* localPlayer = gameManager->getPlayer();
    if (localPlayer == nullptr) {
        return;
    }

    // Rendering boats
    for (auto& [id, boat] : gameManager->getActiveBoats()) {
        boatSprite.setPosition(
            boat->getPosition() - localPlayer->getPosition()
            + sf::Glsl::Vec2(resolution.x * 0.5, resolution.y * 0.5)
        );
        boatSprite.setRotation(sf::degrees(boat->getCurrentAngle()));
        window.draw(boatSprite);
    }
}

void Renderer::render(float time) {
    window.clear();
    renderBackground(time);
    renderBoats();
    debug();
    for(Panel& panel : panels) {
        panel.draw(window);
    }
    if (overlayDrawer) {
        overlayDrawer(window);
    }
    window.display();
}

void Renderer::debug() {
    if (ENV_APP_ENVIRONMENT != 1) return;
    if (gameManager->getSessionPhase() != SessionPhase::Race) return;

    Player* localPlayer = gameManager->getPlayer();
    if (localPlayer == nullptr) {
        return;
    }

    sf::CircleShape colliderCircle(COLLIDER_RADIUS);

    for (auto& [id, boat] : gameManager->getActiveBoats()) {
        colliderCircle.setOrigin({ COLLIDER_RADIUS, COLLIDER_RADIUS });
        colliderCircle.setPosition(
            boat->getPosition() - localPlayer->getPosition()
            + sf::Glsl::Vec2(resolution.x * 0.5, resolution.y * 0.5)
        );

        colliderCircle.setFillColor(sf::Color::Transparent);
        colliderCircle.setOutlineColor(sf::Color::Red);
        colliderCircle.setOutlineThickness(2.f);

        window.draw(colliderCircle);
    }
}
