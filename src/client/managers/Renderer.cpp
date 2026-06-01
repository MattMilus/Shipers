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
      resolution(800.f, 600.f),
      cameraZoom(1.0f) {
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

    // Calculate camera zoom based on local boat speed using a sigmoid mapping:
    // zoom = baseZoom + sigmoid(normSpeed) * maxZoomRange
    float cameraZoom = 1.0f;
    {
        const int playerID = gameManager->getPlayerId();
        Boat* localBoat = gameManager->getBoatById(playerID);
        if (localBoat != nullptr) {
            // Tunable constants
            const float MAX_SPEED = 600.0f; // speed value that maps to 1.0 normalized
            const float SIGMOID_K = 12.0f;   // steepness
            const float SIGMOID_MID = 0.5f;  // midpoint of normalized speed
            const float BASE_ZOOM = 1.0f;    // minimum zoom (no zoom)
            const float MAX_ZOOM_RANGE = 0.6f; // additional zoom at high speed

            float speed = localBoat->getSpeed();
            float norm = speed / MAX_SPEED;
            if (norm < 0.f) norm = 0.f;
            if (norm > 1.f) norm = 1.f;

            float sig = 1.0f / (1.0f + std::exp(-SIGMOID_K * (norm - SIGMOID_MID)));
            cameraZoom = BASE_ZOOM + sig * MAX_ZOOM_RANGE;
        } else {
            cameraZoom = 1.0f;
        }
    }

    waveShader.setUniform("image", renderTex.getTexture());
    waveShader.setUniform("uTime", time);

    // Adjust the camera entry in uPathHistory to account for zoom so shader and SFML
    // remain in sync. Shader computes uv = p0*zoom + (uPathHistory[0]-0.5), which
    // results in the screen center sampling at: uPathHistory[0] + 0.5*(zoom-1).
    // To make the shader sample the actual camera position at the screen center,
    // we send an adjusted uPathHistory[0] = camera_norm - 0.5*(zoom-1).
    std::array<sf::Glsl::Vec2, TOTAL_HISTORY_SIZE> adjustedPathHistory;
    for (int i = 0; i < TOTAL_HISTORY_SIZE; ++i) adjustedPathHistory[i] = uPathHistory[i];
    float zoomOffset = 0.5f * (cameraZoom - 1.0f);
    adjustedPathHistory[0] = sf::Glsl::Vec2(uPathHistory[0].x - zoomOffset, uPathHistory[0].y - zoomOffset);
    waveShader.setUniformArray("uPathHistory", adjustedPathHistory.data(), TOTAL_HISTORY_SIZE);
    // persist zoom for SFML drawing
    this->cameraZoom = cameraZoom;
    waveShader.setUniform("uCameraZoom", cameraZoom);

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
    const float invZoom = 1.0f / cameraZoom;

    // Draw coins
    const auto& coinGroups = gameManager->getCoinGroups();
    sf::CircleShape coinOuterShape;
    sf::CircleShape coinInnerShape;
    coinOuterShape.setOutlineColor(sf::Color::Black);
    coinOuterShape.setOutlineThickness(3.f);
    coinInnerShape.setFillColor(sf::Color::Yellow);

    for (const auto& group : coinGroups) {
        for (const auto& coin : group) {
            if (!coin.isActive()) continue;
            const float radius = coin.getRadius();
            // Apply zoom transform: scale world-space offset by invZoom around screen center
            sf::Vector2f rel = coin.getPosition() - cameraPosition;
            sf::Vector2f screenPos = rel * invZoom + sf::Vector2f(resolution.x * 0.5f, resolution.y * 0.5f);
            const float marginCoin = 50.0f;
            if (screenPos.x < -marginCoin || screenPos.x > resolution.x + marginCoin ||
                screenPos.y < -marginCoin || screenPos.y > resolution.y + marginCoin) {
                continue;
            }
            float drawRadius = radius * invZoom;
            coinOuterShape.setRadius(drawRadius);
            coinOuterShape.setOrigin({ drawRadius, drawRadius });
            coinOuterShape.setPosition(screenPos);
            coinOuterShape.setFillColor(sf::Color(255, 200, 0));
            coinInnerShape.setRadius(drawRadius * 0.55f);
            coinInnerShape.setOrigin({ drawRadius * 0.55f, drawRadius * 0.55f });
            coinInnerShape.setPosition(screenPos);
            coinInnerShape.setFillColor(sf::Color(255, 220, 50));

            window.draw(coinOuterShape);
            window.draw(coinInnerShape);
        }
    }

    sf::CircleShape buoyShape;
    buoyShape.setOrigin({ 25.f, 25.f });
    buoyShape.setFillColor(sf::Color::Yellow);
    buoyShape.setOutlineThickness(3.f);
    buoyShape.setOutlineColor(sf::Color::Black);
    float buoyRadiusOffset = 0.0f;

    const float margin = 50.0f;

    for (const Buoy& buoy : buoys) {
        buoyRadiusOffset += 0.15f;
        sf::Vector2f rel = buoy.position - cameraPosition;
        sf::Vector2f screenPos = rel * invZoom + sf::Vector2f(resolution.x * 0.5f, resolution.y * 0.5f);
        if (screenPos.x < -margin || screenPos.x > resolution.x + margin ||
            screenPos.y < -margin || screenPos.y > resolution.y + margin) {
            continue;
            }
        float drawRadius = (buoy.radius * (0.75 + sin(time + buoyRadiusOffset) * 0.25)) * invZoom;
        buoyShape.setRadius(drawRadius);
        buoyShape.setOrigin({ drawRadius, drawRadius });
        buoyShape.setPosition(screenPos);
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

        // compute screen position with zoom
        sf::Vector2f rel = boat->getPosition() - localPlayer->getPosition();
        sf::Vector2f screenPos = rel * (1.0f / cameraZoom) + sf::Vector2f(resolution.x * 0.5f, resolution.y * 0.5f);

        // scale sprite inversely to camera zoom so objects appear smaller when zoomed out
        const float baseScaleX = 0.5f;
        const float baseScaleY = 0.5f;
        boatSprite.setScale({ baseScaleX * (1.0f / cameraZoom), baseScaleY * (1.0f / cameraZoom) });
        boatSprite.setPosition(screenPos);
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
    // Debug colliders are disabled for players in release overlay.
    // Previously this function drew debug collider outlines per-boat.
    // Kept intentionally empty to hide debug hitboxes for players.
    (void)gameManager;
    (void)resolution;
}
