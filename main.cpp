#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>

using namespace sf;

Vector2f normalize(const Vector2f& source) {
    float length = sqrt((source.x * source.x) + (source.y * source.y));
    if (length != 0)
        return Vector2f(source.x / length, source.y / length);

    return source;
}

int main() {
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

    

    // RenderTexture for the checkerboard (used as input to wave shader)
    RenderTexture renderTex(Vector2u{ 800, 600 });

    RectangleShape screenQuad(Vector2f{ 800.f, 600.f });

    Vector2f boatPos{ 0.5f, 0.5f };
    Vector2 boatSpeed = { 0.f, 0.f };

    Glsl::Vec2 uPathHistory[256];
    for (int i = 255; i >= 0; --i) {
        uPathHistory[i] = boatPos;
    }

    Clock clock;

    checkerShader.setUniform("uResolution", Glsl::Vec2(800.f, 600.f));
    waveShader.setUniform("uResolution", Glsl::Vec2(800.f, 600.f));


    while (window.isOpen()) {
        float time = clock.getElapsedTime().asSeconds();

        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
        }
        Vector2f oldBoatPos = boatPos;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
            boatPos.y += -0.003f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
            boatPos.y +=  0.003f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
            boatPos.x += -0.003f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
            boatPos.x +=  0.003f;

        // TODO: Fix this bullshit
        boatSpeed = oldBoatPos - boatPos;

        boatSprite.setPosition({ boatPos.x * 800.f, boatPos.y * 600.f });
		Angle angle = radians(std::atan2(boatSpeed.y, boatSpeed.x) - 3.1415 / 2);
		boatSprite.setRotation(angle);

        uPathHistory[0] = boatPos;
        for (int i = 255; i > 0; --i) {
            uPathHistory[i] = uPathHistory[i - 1];
        }


        renderTex.clear();
        renderTex.draw(screenQuad, &checkerShader);
        renderTex.display();

        waveShader.setUniform("image", renderTex.getTexture());
        waveShader.setUniform("uTime", time);
        waveShader.setUniformArray("uPathHistory", uPathHistory, 256);

        window.clear();
        window.draw(screenQuad, &waveShader);
        window.draw(boatSprite);
        window.display();
    }

    return 0;
}