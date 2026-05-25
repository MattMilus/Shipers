//
// Created by Wiktor on 25.05.2026.
//

#ifndef SHIPERS_ENDGAMESCOREBOARD_H
#define SHIPERS_ENDGAMESCOREBOARD_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <algorithm>
#include <functional>

struct PlayerScore {
    uint32_t id;
    std::string nickname;
    bool isFinished;
    float finishTime;
    int coins;
    int totalScore;

    bool operator<(const PlayerScore& other) const {
        return totalScore > other.totalScore;
    }
};

class EndGameScoreboard {
private:
    std::vector<PlayerScore> scores;
    std::function<void()> onExitCallback;
    std::function<void()> onPlayAgainCallback;

    sf::RectangleShape btnExit;
    sf::RectangleShape btnPlayAgain;

public:
    EndGameScoreboard();
    void setCallbacks(const std::function<void()> onExit, std::function<void()> onPlayAgain);
    void updateScores(const std::vector<PlayerScore>& newScores);
    void handleMouseClick(const sf::Vector2f& mousePos) const;
    bool containsMouse(const sf::Vector2f& mousePos) const;
    void draw(sf::RenderWindow& renderWindow, const sf::Font& uiFont, uint32_t localPlayerId);
};



#endif //SHIPERS_ENDGAMESCOREBOARD_H
