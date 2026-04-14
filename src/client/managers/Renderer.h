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
#include <functional>

#include "GameManager.h"
#include "../env.h"

#define BOAT_U_PATH_HISTORY_SIZE 256
#define MAX_BOATS 4
#define TOTAL_HISTORY_SIZE (BOAT_U_PATH_HISTORY_SIZE * MAX_BOATS)

class Panel {
private:
    sf::Vector2f position;
    sf::RectangleShape background;
    sf::RectangleShape clickedBackground;

    sf::Text text;
	sf::Text clickedText;

    std::function<void()> clickFunc;
    std::function<void()> hoverFunc;

public:
    Panel();

    bool changeStyle = false; //TODO: getter, setter, name change

    

    void setFontSize(unsigned int size);
    void setText(const char* format, ...);
    std::string getText() { return text.getString(); }
    void setHeldText(const char* format, ...);

    void setPosition(const sf::Vector2f& pos) { 
        background.setPosition(pos); text.setPosition(pos); 
        clickedBackground.setPosition(pos); clickedText.setPosition(pos);
    }
    void setSize(const sf::Vector2f& sz) { background.setSize(sz); clickedBackground.setSize(sz); }

    void setStyle(const sf::Color& txtColor, int fontSize, const sf::Color& bgColor, const sf::Color& bdColor, float thickness) {
        text.setFillColor(txtColor);
		text.setCharacterSize(fontSize);
        background.setFillColor(bgColor);
		background.setOutlineThickness(thickness);
        background.setOutlineColor(bdColor);
    }
	void setHeldStyle(const sf::Color& txtColor, int fontSize, const sf::Color& bgColor, const sf::Color& bdColor, float thickness) {
        clickedText.setFillColor(txtColor);
        clickedText.setCharacterSize(fontSize);
		clickedBackground.setFillColor(bgColor);
        clickedBackground.setOutlineColor(bdColor);
		clickedBackground.setOutlineThickness(thickness);
	}
	void switchStyle() { 
        changeStyle = !changeStyle;
        std::swap(background, clickedBackground); std::swap(text, clickedText); 
    }

    void setButton(std::function <void()> func) { clickFunc.swap(func); } // Who will ever unmake a button? :P
    void setHover(std::function <void()> func) { hoverFunc.swap(func); }

	void onClick() { if (clickFunc) clickFunc(); }
	void onHover() { if (hoverFunc) hoverFunc(); }

	bool contains(const sf::Vector2f& point) const { return background.getGlobalBounds().contains(point); } 


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
    size_t getPanelIdAt(const sf::Vector2f& pos);
    void releaseAllButtons();
    void debug();
};



#endif //RENDERER_H
