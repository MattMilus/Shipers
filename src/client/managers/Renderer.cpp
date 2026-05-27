//
// Created by Wiktor on 12.03.2026.
//

#include "Renderer.h"

#include <cmath>
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

sf::RenderWindow* Renderer::initialize() {
    window.create(sf::VideoMode({ 800, 600 }), "GPU Ripples");
    window.setFramerateLimit(60);

    loadShaders();
    loadTextures();
    loadSprites();

    if (!renderTex.resize(sf::Vector2u{ 800, 600 })) {
        std::cerr << "Failed to create/resize render texture\n";
    }
    screenQuad.setSize(sf::Vector2f{ 800.f, 600.f });

    for (int i = TOTAL_HISTORY_SIZE - 1; i >= 0; --i) {         // "co z oczu to z serca..."
        uPathHistory[i] = sf::Glsl::Vec2(10000.0f, 10000.0f);   // czemu nie dasz po prostu FLT_MAX?
    }                                                           // bo wtedy fale laduja sie niewiadomo ile
    uPathHistory[0] = sf::Glsl::Vec2(0.f, 0.f); // "...ale nie kamera"

    return &window;
}

void Renderer::renderBackground(float time) {
    const auto& boats = gameManager->getActiveBoats();
    const int boatCount = static_cast<int>(boats.size());

    if (boatCount > 0) {
        const int playerID = gameManager->getPlayerId();
        int currentIndex = 1;

        Boat* localBoat = gameManager->getBoatById(playerID);
        if (localBoat != nullptr) {
            sf::Vector2f pos = localBoat->getPosition();
            uPathHistory[0] = sf::Glsl::Vec2(pos.x / 800.f, pos.y / 600.f);
        } else {
            currentIndex = 0;
        }

        for (const auto& [id, boat] : boats) {
            if (id == playerID && localBoat != nullptr) {
                continue;
            }
            if (boat->isFinished()) continue;

            sf::Vector2f pos = boat->getPosition();
            // Zabezpieczenie przed przepełnieniem (na wypadek dziwnych błędów ilości łodzi)
            if (currentIndex < BOAT_U_PATH_HISTORY_SIZE) {
                uPathHistory[currentIndex] = sf::Glsl::Vec2(pos.x / 800.f, pos.y / 600.f);
                currentIndex++;
            }
        }

        for (int i = BOAT_U_PATH_HISTORY_SIZE - 1; i >= boatCount; --i) {
            uPathHistory[i] = uPathHistory[i - boatCount];
        }
    }

    const auto& buoys = gameManager->getTrack().getBuoys();

    // ==========================================
    // KLUCZOWA ZMIANA: Zabezpieczenie pętli buoys
    // ==========================================
    // Wyliczamy, ile miejsca zostało nam do końca tablicy uPathHistory.
    int availableBuoySlots = TOTAL_HISTORY_SIZE - BOAT_U_PATH_HISTORY_SIZE;
    // Pętla obróci się maksymalnie tyle razy, ile mamy wolnych slotów w tablicy LUB tyle ile jest boi.
    int limit = std::min(static_cast<int>(buoys.size()), availableBuoySlots);

    for (int i = 0; i < limit; ++i) {
        uPathHistory[i + BOAT_U_PATH_HISTORY_SIZE] = buoys[i].position;
    }
    // Ewentualne wyczyszczenie "resztek" jeśli tras było wcześniej więcej, a potem mniej
    /*for(int i = limit; i < availableBuoySlots; ++i) {
        uPathHistory[i + BOAT_U_PATH_HISTORY_SIZE] = sf::Glsl::Vec2(10000.0f, 10000.0f);
    }*/

    renderTex.clear();
    renderTex.draw(screenQuad, &checkerShader);
    renderTex.display();

    waveShader.setUniform("image", renderTex.getTexture());
    waveShader.setUniform("uTime", time);
    waveShader.setUniformArray("uPathHistory", uPathHistory, TOTAL_HISTORY_SIZE);

    window.clear();
    window.draw(screenQuad, &waveShader);

    renderFinish();
}


void Renderer::renderFinish() {
    sf::Vector2f finishPos = gameManager->getTrack().getFinishBuoy().position;
    float finishRadius = gameManager->getTrack().getFinishRadius();

    Player* localPlayer = gameManager->getPlayer();
    if (localPlayer == nullptr) {
        return;
    }

    sf::Vector2f screenFinishPos = finishPos - localPlayer->getPosition() + sf::Vector2f(resolution.x * 0.5f, resolution.y * 0.5f);

    int segments = 40;
    sf::VertexArray dashedCircle(sf::PrimitiveType::Lines);

    for (int i = 0; i < segments; i += 2) {
        float angle1 = i * (2.0f * 3.14159f) / segments;
        float angle2 = (i + 1) * (2.0f * 3.14159f) / segments;

        sf::Vector2f p1 = screenFinishPos + sf::Vector2f(std::cos(angle1) * finishRadius, std::sin(angle1) * finishRadius);
        sf::Vector2f p2 = screenFinishPos + sf::Vector2f(std::cos(angle2) * finishRadius, std::sin(angle2) * finishRadius);

        dashedCircle.append(sf::Vertex{p1, sf::Color::Black});
        dashedCircle.append(sf::Vertex{p2, sf::Color::Black});
    }
    window.draw(dashedCircle);

    float buoyVisualRadius = 25.f;
    sf::CircleShape finishBuoyShape(buoyVisualRadius);
    finishBuoyShape.setFillColor(sf::Color::Black);
    finishBuoyShape.setOrigin({buoyVisualRadius, buoyVisualRadius});
    finishBuoyShape.setPosition(screenFinishPos);

    window.draw(finishBuoyShape);
}

void Renderer::renderTrack(float time) {
    if (gameManager->getSessionPhase() != SessionPhase::Race) {
        return;
    }

	const std::vector<Buoy>& buoys = gameManager->getTrack().getBuoys();

	const sf::Vector2f cameraPosition = gameManager->getPlayer()->getPosition();
	
	sf::CircleShape buoyShape;
	buoyShape.setOrigin({ 25.f, 25.f });
	buoyShape.setFillColor(sf::Color::Yellow);
	buoyShape.setOutlineThickness(3.f);
	buoyShape.setOutlineColor(sf::Color::Black);
	float buoyRadiusOffset = 0.0f;
	for (const Buoy& buoy : buoys) {
        buoyShape.setRadius(buoy.radius * (0.75 + sin(time + buoyRadiusOffset) * 0.25));
		buoyRadiusOffset += 0.15f;
		buoyShape.setPosition(
			buoy.position - cameraPosition
			+ sf::Glsl::Vec2(resolution.x * 0.5f, resolution.y * 0.5f)
		);
		window.draw(buoyShape);
	}
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
        if (boat->isFinished()) continue;

        boatSprite.setPosition(
            boat->getPosition() - localPlayer->getPosition()
            + sf::Glsl::Vec2(resolution.x * 0.5f, resolution.y * 0.5f)
        );
        boatSprite.setRotation(sf::degrees(boat->getCurrentAngle()));
        window.draw(boatSprite);
    }
}

void Renderer::render(float time) {
    window.clear();
    renderBackground(time);
    renderBoats();
	renderTrack(time);
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
        if (boat->isFinished()) continue;
        colliderCircle.setOrigin({ COLLIDER_RADIUS, COLLIDER_RADIUS });
        colliderCircle.setPosition(
            boat->getPosition() - localPlayer->getPosition()
            + sf::Glsl::Vec2(resolution.x * 0.5f, resolution.y * 0.5f)
        );

        colliderCircle.setFillColor(sf::Color::Transparent);
        colliderCircle.setOutlineColor(sf::Color::Red);
        colliderCircle.setOutlineThickness(2.f);

        window.draw(colliderCircle);
    }
}
