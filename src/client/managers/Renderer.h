//
// Created by Wiktor on 12.03.2026.
//

#ifndef RENDERER_H
#define RENDERER_H
#include <SFML/Graphics/Glsl.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <functional>
#include <utility>

#include "GameManager.h"
#include "../env.h"
#include "Panel.h"

#define BOAT_U_PATH_HISTORY_SIZE 256
#define BOUY_U_PATH_HISTORY_SIZE 256
#define TOTAL_HISTORY_SIZE (BOAT_U_PATH_HISTORY_SIZE + BOUY_U_PATH_HISTORY_SIZE)


class Renderer {
private:
    GameManager* gameManager;

    sf::RenderWindow window;
    sf::Glsl::Vec2 resolution;


    sf::Glsl::Vec2 uPathHistory[TOTAL_HISTORY_SIZE];
    sf::Shader checkerShader;
    sf::Shader waveShader;
    sf::Texture boatTexture;
    sf::Sprite boatSprite;
    sf::RenderTexture renderTex;
    sf::RectangleShape screenQuad;

    std::vector<Panel> panels;
    std::function<void(sf::RenderWindow&)> overlayDrawer;

    void loadShaders();
    void loadTextures();
    void loadSprites();
protected:
    void renderBackground(float time);
    void renderBoats();
    void renderTrack();
public:
    Renderer(GameManager* game_manager);

    sf::RenderWindow* initialize();

    void render(float time);
    void setOverlayDrawer(std::function<void(sf::RenderWindow&)> drawer) { overlayDrawer = std::move(drawer); }
    size_t addPanel() { panels.emplace_back(); return panels.size() - 1; }
    Panel& getPanel(size_t index) { return panels.at(index); }
    size_t getPanelIdAt(const sf::Vector2f& pos);
    void releaseAllButtons();
    void debug();
};



#endif //RENDERER_H
