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
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>

#include "GameManager.h"
#include "../env.h"

#define BOAT_U_PATH_HISTORY_SIZE 256
#define MAX_BOATS 4
#define TOTAL_HISTORY_SIZE (BOAT_U_PATH_HISTORY_SIZE * MAX_BOATS)


class Panel {
private:
    sf::Vector2f position;
    sf::RectangleShape background;

    sf::Text text;

    bool isClickable;

public:
    Panel();

    void setFont(const std::string& fontPath);
    void setFontSize(unsigned int size);
    void setText(const char* format, ...);

    void setPosition(const sf::Vector2f& pos) { background.setPosition(pos); text.setPosition(pos); }
    void setSize(const sf::Vector2f& sz) { background.setSize(sz); }
    void setTextColor(const sf::Color& color) { text.setFillColor(color); }
    void setBackgroundColor(const sf::Color& color) { background.setFillColor(color); }
    void setBorderThickness(float thickness) { background.setOutlineThickness(thickness); }
    void setBorderColor(const sf::Color& color) { background.setOutlineColor(color); }
    void setClickable(bool clickable) { isClickable = clickable; }
    void draw(sf::RenderWindow& window);
};

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

    void loadShaders();
    void loadTextures();
    void loadSprites();
protected:
    void renderBackground(float time);
    void renderBoats();
public:
    Renderer(GameManager* game_manager);

    sf::RenderWindow* initialize();

    void render(float time);
    size_t addPanel() { panels.emplace_back(); return panels.size() - 1; }
    Panel& getPanel(size_t index) { return panels.at(index); }
    void debug();
};



#endif //RENDERER_H
