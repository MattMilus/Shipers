#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <vector>
#include <cmath>
#include <iostream>
#include <iomanip>

// Struktura przechowująca pojedynczą linię/trasę
struct TrackPath {
    std::vector<sf::Vector2f> points;
    bool isDouble = true; // true = Trasa (2 bandy), false = Pojedyncza ściana/bariera
};

sf::Vector2f getBSplinePoint(const sf::Vector2f& p0, const sf::Vector2f& p1,
                             const sf::Vector2f& p2, const sf::Vector2f& p3, float t) {
    float it = 1.0f - t;
    float t2 = t * t;
    float t3 = t2 * t;

    float b0 = (it * it * it) / 6.0f;
    float b1 = (3.0f * t3 - 6.0f * t2 + 4.0f) / 6.0f;
    float b2 = (-3.0f * t3 + 3.0f * t2 + 3.0f * t + 1.0f) / 6.0f;
    float b3 = t3 / 6.0f;

    return {
        b0 * p0.x + b1 * p1.x + b2 * p2.x + b3 * p3.x,
        b0 * p0.y + b1 * p1.y + b2 * p2.y + b3 * p3.y
    };
}

sf::Vector2f normalize(const sf::Vector2f& source) {
    float length = std::hypot(source.x, source.y);
    if (length != 0) return { source.x / length, source.y / length };
    return source;
}

int main() {
    sf::RenderWindow window(sf::VideoMode({1600, 900}), "Zaawansowany Edytor Trasy i Band");
    window.setFramerateLimit(60);

    sf::View view = window.getDefaultView();
    float zoomLevel = 1.0f;

    // Główny kontener na wszystkie ścieżki
    std::vector<TrackPath> paths;
    // Dodajemy pierwszą, domyślną trasę
    paths.push_back({ {{0.0f, 0.0f}, {300.0f, 700.0f}, {1000.0f, 1000.0f}}, true });

    int activePathIndex = 0; // Indeks aktualnie edytowanej ścieżki
    sf::Vector2f finishPos(1200.0f, 1200.0f);

    float trackWidth = 80.0f;
    int segmentsPerCurve = 20;

    int draggedPathIndex = -1;
    int draggedPointIndex = -1;
    bool isDragging = false;
    bool editFinishMode = false;

    std::cout << "=== INSTRUKCJA EDYTORA ===\n";
    std::cout << "[Lewy Myszy] : Dodaj punkt / Przeciagaj punkt\n";
    std::cout << "[Prawy Myszy]: Usun punkt kontrolny\n";
    std::cout << "[WSAD]       : Przesuwanie widoku kamery\n";
    std::cout << "[Scroll]     : Przyblizanie / Oddalanie\n";
    std::cout << "[N]          : NOWA linia/sciezka\n";
    std::cout << "[TAB]        : ZMIEN aktywna linie\n";
    std::cout << "[T]          : PRZELACZ tryb aktywnej linii (Podwojna Trasa <-> Pojedyncza Banda)\n";
    std::cout << "[F]          : Przestaw METE\n";
    std::cout << "[C]          : Wyczysc wszystko\n";
    std::cout << "[SPACE]      : EKSPORT KODU C++ DO KONSOLI\n\n";

    while (window.isOpen()) {
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            sf::Vector2f mouseWorldPos = window.mapPixelToCoords(sf::Mouse::getPosition(window), view);

            if (const auto* scroll = event->getIf<sf::Event::MouseWheelScrolled>()) {
                if (scroll->delta > 0) { view.zoom(0.9f); zoomLevel *= 0.9f; }
                else { view.zoom(1.1f); zoomLevel *= 1.1f; }
            }

            if (const auto* mouseBtn = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mouseBtn->button == sf::Mouse::Button::Left) {
                    if (editFinishMode) {
                        finishPos = mouseWorldPos;
                        editFinishMode = false;
                        std::cout << "Meta przestawiona.\n";
                    } else {
                        float catchRadius = 15.0f * zoomLevel;
                        bool caught = false;

                        // Szukamy czy kliknięto w jakikolwiek punkt na dowolnej ścieżce
                        for (size_t p = 0; p < paths.size(); ++p) {
                            for (size_t i = 0; i < paths[p].points.size(); ++i) {
                                if (std::hypot(paths[p].points[i].x - mouseWorldPos.x, paths[p].points[i].y - mouseWorldPos.y) < catchRadius) {
                                    draggedPathIndex = static_cast<int>(p);
                                    draggedPointIndex = static_cast<int>(i);
                                    activePathIndex = static_cast<int>(p); // Złapanie punktu aktywuje jego ścieżkę
                                    isDragging = true;
                                    caught = true;
                                    break;
                                }
                            }
                            if (caught) break;
                        }

                        // Jeśli kliknięto w puste miejsce, dodajemy punkt do aktywnej ścieżki
                        if (!caught && !paths.empty()) {
                            paths[activePathIndex].points.push_back(mouseWorldPos);
                        }
                    }
                }
                else if (mouseBtn->button == sf::Mouse::Button::Right) {
                    float catchRadius = 15.0f * zoomLevel;
                    bool deleted = false;
                    for (size_t p = 0; p < paths.size(); ++p) {
                        for (auto it = paths[p].points.begin(); it != paths[p].points.end(); ++it) {
                            if (std::hypot(it->x - mouseWorldPos.x, it->y - mouseWorldPos.y) < catchRadius) {
                                paths[p].points.erase(it);
                                deleted = true;
                                break;
                            }
                        }
                        if (deleted) break;
                    }
                }
            }

            if (const auto* mouseRel = event->getIf<sf::Event::MouseButtonReleased>()) {
                if (mouseRel->button == sf::Mouse::Button::Left) {
                    isDragging = false;
                    draggedPathIndex = -1;
                    draggedPointIndex = -1;
                }
            }

            if (event->is<sf::Event::MouseMoved>() && isDragging && draggedPathIndex != -1 && draggedPointIndex != -1) {
                paths[draggedPathIndex].points[draggedPointIndex] = mouseWorldPos;
            }

            if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                if (key->code == sf::Keyboard::Key::N) {
                    paths.push_back({ {}, true });
                    activePathIndex = static_cast<int>(paths.size() - 1);
                    std::cout << "Dodano nowa sciezke. Aktywna sciezka: " << activePathIndex << "\n";
                }
                if (key->code == sf::Keyboard::Key::Tab) {
                    if (!paths.empty()) {
                        activePathIndex = (activePathIndex + 1) % paths.size();
                        std::cout << "Aktywna sciezka zmieniona na: " << activePathIndex << "\n";
                    }
                }
                if (key->code == sf::Keyboard::Key::T) {
                    if (!paths.empty()) {
                        paths[activePathIndex].isDouble = !paths[activePathIndex].isDouble;
                        std::cout << "Zmieniono typ sciezki " << activePathIndex << " na: "
                                  << (paths[activePathIndex].isDouble ? "Podwojna Trasa" : "Pojedyncza Banda") << "\n";
                    }
                }
                if (key->code == sf::Keyboard::Key::C) {
                    paths.clear();
                    paths.push_back({ {}, true });
                    activePathIndex = 0;
                    std::cout << "Wyczyszczono wszystkie punkty i sciezki.\n";
                }
                if (key->code == sf::Keyboard::Key::F) {
                    editFinishMode = true;
                }
                if (key->code == sf::Keyboard::Key::Space) {
                    std::cout << "\n--- SKOPIUJ PONIZSZY KOD DO SWOJEJ GRY ---\n";
                    for (size_t p = 0; p < paths.size(); ++p) {
                        if (paths[p].points.empty()) continue;

                        if (paths[p].isDouble) {
                            std::cout << "// TRASA GLOWNA (Podwojna)\n";
                            std::cout << "gameManager->generateTrack({ ";
                        } else {
                            std::cout << "// BARIERA / ZABEZPIECZENIE (Pojedyncza)\n";
                            std::cout << "gameManager->generateBarrier({ "; // Pamiętaj by dodać taką funkcję w swoim kodzie gry!
                        }

                        for (size_t i = 0; i < paths[p].points.size(); ++i) {
                            std::cout << "{" << std::fixed << std::setprecision(1) << paths[p].points[i].x << "f, " << paths[p].points[i].y << "f}";
                            if (i < paths[p].points.size() - 1) std::cout << ", ";
                        }

                        if (paths[p].isDouble) {
                            std::cout << " }, {" << finishPos.x << "f, " << finishPos.y << "f});\n\n";
                        } else {
                            std::cout << " });\n\n";
                        }
                    }
                    std::cout << "------------------------------------------\n";
                }
            }
        }

        float cameraSpeed = 400.0f * zoomLevel;
        float dt = 1.0f / 60.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) view.move({0, -cameraSpeed * dt});
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) view.move({0, cameraSpeed * dt});
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) view.move({-cameraSpeed * dt, 0});
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) view.move({cameraSpeed * dt, 0});

        window.clear(sf::Color(30, 30, 35));
        window.setView(view);

        // --- Renderowanie wszystkich ścieżek ---
        for (size_t p = 0; p < paths.size(); ++p) {
            const auto& path = paths[p];
            bool isActive = (p == activePathIndex);

            // Obliczanie punktów B-Spline dla danej ścieżki
            std::vector<sf::Vector2f> splinePoints;
            if (path.points.size() >= 2) {
                std::vector<sf::Vector2f> paddedPoints;
                paddedPoints.push_back(path.points.front());
                paddedPoints.push_back(path.points.front());
                for (const auto& pt : path.points) paddedPoints.push_back(pt);
                paddedPoints.push_back(path.points.back());
                paddedPoints.push_back(path.points.back());

                for (size_t i = 0; i < paddedPoints.size() - 3; ++i) {
                    for (int j = 0; j <= segmentsPerCurve; ++j) {
                        if (j == 0 && i > 0) continue;
                        sf::Vector2f pos = getBSplinePoint(
                            paddedPoints[i], paddedPoints[i + 1],
                            paddedPoints[i + 2], paddedPoints[i + 3],
                            static_cast<float>(j) / segmentsPerCurve
                        );
                        splinePoints.push_back(pos);
                    }
                }
            }

            // Rysowanie szkieletu i punktów kontrolnych
            if (!path.points.empty()) {
                sf::VertexArray skeleton(sf::PrimitiveType::LineStrip, path.points.size());
                for (size_t i = 0; i < path.points.size(); ++i) {
                    skeleton[i].position = path.points[i];
                    skeleton[i].color = isActive ? sf::Color(150, 150, 150, 150) : sf::Color(80, 80, 80, 80);

                    sf::CircleShape cpShape(isActive ? 8.0f * zoomLevel : 6.0f * zoomLevel);
                    cpShape.setOrigin({cpShape.getRadius(), cpShape.getRadius()});
                    cpShape.setPosition(path.points[i]);
                    // Aktywna ścieżka ma jasnoczerwone punkty, nieaktywna ciemnobordowe
                    cpShape.setFillColor(isActive ? sf::Color(235, 94, 85) : sf::Color(120, 40, 40));
                    window.draw(cpShape);
                }
                window.draw(skeleton);
            }

            // Rysowanie wygładzonej linii (Trasa vs Banda)
            if (splinePoints.size() >= 2) {
                if (path.isDouble) {
                    // Tryb: PODWÓJNA TRASA
                    sf::VertexArray centerLine(sf::PrimitiveType::LineStrip, splinePoints.size());
                    sf::VertexArray leftBanda(sf::PrimitiveType::LineStrip, splinePoints.size());
                    sf::VertexArray rightBanda(sf::PrimitiveType::LineStrip, splinePoints.size());

                    for (size_t i = 0; i < splinePoints.size(); ++i) {
                        centerLine[i].position = splinePoints[i];
                        centerLine[i].color = sf::Color(255, 255, 255, isActive ? 80 : 30);

                        sf::Vector2f dir;
                        if (i < splinePoints.size() - 1) dir = splinePoints[i + 1] - splinePoints[i];
                        else dir = splinePoints[i] - splinePoints[i - 1];

                        sf::Vector2f normal = normalize({ -dir.y, dir.x });

                        leftBanda[i].position = splinePoints[i] + normal * trackWidth;
                        leftBanda[i].color = isActive ? sf::Color(52, 152, 219) : sf::Color(30, 80, 120);

                        rightBanda[i].position = splinePoints[i] - normal * trackWidth;
                        rightBanda[i].color = isActive ? sf::Color(46, 204, 113) : sf::Color(20, 100, 50);
                    }
                    window.draw(centerLine);
                    window.draw(leftBanda);
                    window.draw(rightBanda);
                } else {
                    // Tryb: POJEDYNCZA BANDA
                    sf::VertexArray solidBarrier(sf::PrimitiveType::LineStrip, splinePoints.size());
                    for (size_t i = 0; i < splinePoints.size(); ++i) {
                        solidBarrier[i].position = splinePoints[i];
                        solidBarrier[i].color = isActive ? sf::Color(243, 156, 18) : sf::Color(120, 80, 10); // Pomarańczowy kolor bariery
                    }
                    window.draw(solidBarrier);
                }
            }
        }

        sf::CircleShape finishShape(15.0f * zoomLevel);
        finishShape.setOrigin({finishShape.getRadius(), finishShape.getRadius()});
        finishShape.setPosition(finishPos);
        finishShape.setFillColor(sf::Color::Yellow);
        window.draw(finishShape);

        window.display();
    }

    return 0;
}