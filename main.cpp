#include <SFML/Graphics.hpp>
#include <optional>
#include <iostream>
#include <vector>
#include <string>


using namespace sf;
using std::optional;
using std::string;
using std::vector;
using std::cerr;
using std::string;
using std::to_string;

struct circleData {
    float x, y;
    float r, g, b;
    float radius;
};

struct Circle {
    Vector2f position;
	Vector3f color;
	float lifetime;
	Circle() : position(0, 0), color(0, 0, 0), lifetime(0) {}
	Circle(Vector2f position, Vector3f color, float lifetime) : position(position), color(color), lifetime(lifetime) {}
};

int main() {
	std::cout << "Starting application...\n";
    RenderWindow window(VideoMode({ 800, 600 }), "Rayman");
    window.setFramerateLimit(60);

    Shader shader;
    if (!shader.loadFromFile("shader.frag", Shader::Type::Fragment)) {
        cerr << "Failed to load shader\n";
        return -1;
    }
	std::cout << "Shader loaded successfully.\n";

    RectangleShape screenQuad;
    screenQuad.setSize({ 800, 800 });


	Vector2f boatPos = { 0.5f , 0.5f };

	Circle defaultCircle(boatPos, Vector3f(0.6f, 0.7f, 1.0f), 0.1f);
    vector<Circle> circles(64, defaultCircle);


    int size = circles.size();
    shader.setUniform("uResolution", Glsl::Vec2(800, 800));


	float maxLifeTime = 1.0f;

    shader.setUniform("uCount", size);
    for(size_t i = 1; i < size; ++i) {
        string it = to_string(i);

        circles[i].lifetime += maxLifeTime * i / ((float)size - 1.0f);
        shader.setUniform("uPositions[" + it + "]", circles[i].position);
        shader.setUniform("uColors[" + it + "]", circles[i].color);
        shader.setUniform("uLifetimes[" + it + "]", circles[i].lifetime);
	}
    shader.setUniform("uLifetimes[0]", circles[0].lifetime);
    
    for(float time = 0.0f; window.isOpen(); time = (float)clock() / CLOCKS_PER_SEC) {
        while (const optional event = window.pollEvent()) {
            if (event->is<Event::Closed>())
                window.close();
        }

        if(Keyboard::isKeyPressed(sf::Keyboard::Key::W))
			boatPos.y += 0.5f * (1.0f / 60.0f);
        if (Keyboard::isKeyPressed(sf::Keyboard::Key::S))
            boatPos.y -= 0.5f * (1.0f / 60.0f);
        if (Keyboard::isKeyPressed(sf::Keyboard::Key::A))
            boatPos.x -= 0.5f * (1.0f / 60.0f);
        if (Keyboard::isKeyPressed(sf::Keyboard::Key::D))
            boatPos.x += 0.5f * (1.0f / 60.0f);

        shader.setUniform("uTime", time);

        shader.setUniform("uPositions[0]", boatPos);

        for(int i = 1; i < size; ++i) {
            string it = to_string(i);

			circles[i].lifetime += 1.0f / 60.0f;

            if (circles[i].lifetime > maxLifeTime) {
                shader.setUniform("uPositions[" + it + "]", boatPos);
                circles[i].lifetime = 0.0;
            }

            shader.setUniform("uLifetimes[" + it + "]", circles[i].lifetime);
		}


        window.clear();
        window.draw(screenQuad, &shader);
        window.display();
    }
    return 0;
}