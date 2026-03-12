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

#include "GameManager.h"

#define BOAT_U_PATH_HISTORY_SIZE 256
#define MAX_BOATS 4
#define TOTAL_HISTORY_SIZE (BOAT_U_PATH_HISTORY_SIZE * MAX_BOATS)


class Renderer {
private:
    GameManager* gameManager;

    sf::RenderWindow window;
    sf::Glsl::Vec2 uPathHistory[TOTAL_HISTORY_SIZE];
    sf::Shader checkerShader;
    sf::Shader waveShader;
    sf::Texture boatTexture;
    sf::Sprite boatSprite;
    sf::RenderTexture renderTex;
    sf::RectangleShape screenQuad;


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
};



#endif //RENDERER_H
