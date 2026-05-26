//
// Created by Wiktor on 25.05.2026.
//

#include "EndGameScoreboard.h"

EndGameScoreboard::EndGameScoreboard() {
    // Inicjalizacja przycisku "Wyjdź z gry"
    btnExit.setSize({180.0f, 50.0f});
    btnExit.setFillColor(sf::Color(180, 50, 50, 235));
    btnExit.setOutlineColor(sf::Color(255, 100, 100));
    btnExit.setOutlineThickness(2.0f);

    // Inicjalizacja przycisku "Zagraj ponownie"
    btnPlayAgain.setSize({220.0f, 50.0f});
    btnPlayAgain.setFillColor(sf::Color(34, 112, 73, 235));
    btnPlayAgain.setOutlineColor(sf::Color(80, 180, 120));
    btnPlayAgain.setOutlineThickness(2.0f);
}

void EndGameScoreboard::setCallbacks(std::function<void()> onExit, std::function<void()> onPlayAgain) {
    onExitCallback = std::move(onExit);
    onPlayAgainCallback = std::move(onPlayAgain);
}

void EndGameScoreboard::updateScores(const std::vector<PlayerScore>& newScores) {
    scores = newScores;
    std::sort(scores.begin(), scores.end());
}

void EndGameScoreboard::handleMouseClick(const sf::Vector2f& mousePos) const {
    if (btnExit.getGlobalBounds().contains(mousePos) && onExitCallback) {
        onExitCallback();
    } else if (btnPlayAgain.getGlobalBounds().contains(mousePos) && onPlayAgainCallback) {
        onPlayAgainCallback();
    }
}

bool EndGameScoreboard::containsMouse(const sf::Vector2f& mousePos) const {
    return btnExit.getGlobalBounds().contains(mousePos) ||
           btnPlayAgain.getGlobalBounds().contains(mousePos);
}

void EndGameScoreboard::draw(sf::RenderWindow& renderWindow, const sf::Font& uiFont, uint32_t localPlayerId) {
    const sf::Vector2f windowSize(
        static_cast<float>(renderWindow.getSize().x),
        static_cast<float>(renderWindow.getSize().y)
    );

    // --- 1. TŁO (OVERLAY) ---
    sf::RectangleShape overlay(windowSize);
    overlay.setFillColor(sf::Color(9, 18, 30, 214));
    renderWindow.draw(overlay);

    // Szukanie miejsca lokalnego gracza
    int localPlayerPlace = 0;
    for (size_t i = 0; i < scores.size(); ++i) {
        if (scores[i].id == localPlayerId) {
            localPlayerPlace = i + 1;
            break;
        }
    }

    // --- 2. PASEK TYTUŁOWY I NAPIS ---
    sf::RectangleShape titleBand({windowSize.x, 80.0f});
    titleBand.setFillColor(sf::Color(20, 44, 70, 235));
    renderWindow.draw(titleBand);

    sf::Text title(uiFont);
    title.setCharacterSize(32);
    title.setFillColor(sf::Color::White);
    if (localPlayerPlace > 0) {
        title.setString("Koniec gry, zajales " + std::to_string(localPlayerPlace) + " miejsce!");
    } else {
        title.setString("Koniec gry!");
    }

    // Wyśrodkowanie tytułu
    const sf::FloatRect titleBounds = title.getLocalBounds();
    title.setPosition({(windowSize.x - titleBounds.size.x) / 2.0f, 20.0f});
    renderWindow.draw(title);

    // --- 3. NAGŁÓWEK TABELI ---
    const float tableLeft = 48.0f;
    const float tableTop = 150.0f;
    const float tableWidth = windowSize.x - (tableLeft * 2.0f);
    const float headerHeight = 48.0f;
    const float rowHeight = 62.0f;

    // Ustawienie X kolumn dla proporcjonalnego odstępu
    const float colPlaceX = tableLeft + 24.0f;
    const float colNickX = tableLeft + 100.0f;
    const float colTimeX = tableLeft + 300.0f; // Cofnięte nieco w lewo
    const float colCoinsX = tableLeft + 440.0f; // Zdecydowanie cofnięte w lewo
    const float colScoreX = tableLeft + tableWidth - 170.0f;

    sf::RectangleShape header({tableWidth, headerHeight});
    header.setPosition({tableLeft, tableTop});
    header.setFillColor(sf::Color(30, 63, 97, 245));
    header.setOutlineColor(sf::Color(214, 227, 247));
    header.setOutlineThickness(2.0f);
    renderWindow.draw(header);

    auto drawCellText = [&](const std::string& str, float x, float y, int size = 18, sf::Color color = sf::Color::White) {
        sf::Text text(uiFont);
        text.setCharacterSize(size);
        text.setFillColor(color);
        text.setString(str);
        text.setPosition({x, y});
        renderWindow.draw(text);
    };

    float headerY = tableTop + 11.0f;
    drawCellText("Lp.", colPlaceX, headerY);
    drawCellText("Nickname", colNickX, headerY);
    drawCellText("Czas", colTimeX, headerY);
    drawCellText("Monety", colCoinsX, headerY);
    drawCellText("Suma Punktow", colScoreX, headerY);

    // --- 4. WIERSZE TABELI ---
    float currentRowTop = tableTop + headerHeight + 12.0f;

    for (size_t i = 0; i < scores.size(); ++i) {
        const auto& scoreInfo = scores[i];

        sf::RectangleShape row({tableWidth, rowHeight});
        row.setPosition({tableLeft, currentRowTop});
        row.setFillColor(i % 2 == 0 ? sf::Color(241, 246, 252, 232) : sf::Color(226, 236, 248, 232));
        row.setOutlineColor(sf::Color(191, 206, 226));
        row.setOutlineThickness(1.0f);

        // Podświetlenie wiersza lokalnego gracza
        if (scoreInfo.id == localPlayerId) {
            row.setFillColor(sf::Color(255, 235, 153, 232));
            row.setOutlineColor(sf::Color(255, 204, 0));
            row.setOutlineThickness(2.0f);
        }
        renderWindow.draw(row);

        float cellY = currentRowTop + 18.0f;
        sf::Color textColor(20, 35, 55);

        // Miejsce
        drawCellText(std::to_string(i + 1), colPlaceX, cellY, 20, textColor);

        // Nick (dodanie napisu "(you)")
        std::string dispNick = scoreInfo.nickname;
        if (scoreInfo.id == localPlayerId) dispNick += " (you)";
        drawCellText(dispNick, colNickX, cellY, 20, textColor);

        // Czas i status
        std::string timeStr = scoreInfo.isFinished ? (std::to_string(scoreInfo.finishTime).substr(0, 5) + "s") : "W wyscigu";
        sf::Color timeColor = scoreInfo.isFinished ? sf::Color(34, 112, 73) : sf::Color(145, 92, 28);
        drawCellText(timeStr, colTimeX, cellY, 20, timeColor);

        // Monety i Punkty
        drawCellText(std::to_string(scoreInfo.coins), colCoinsX, cellY, 20, textColor);
        drawCellText(std::to_string(scoreInfo.totalScore), colScoreX, cellY, 20, sf::Color(160, 40, 40));

        currentRowTop += rowHeight + 10.0f;
    }

    // --- 5. PRZYCISKI ---
    float buttonsTop = currentRowTop + 20.0f;

    btnExit.setPosition({tableLeft, buttonsTop});
    renderWindow.draw(btnExit);
    drawCellText("Wyjdz z gry", tableLeft + 25.0f, buttonsTop + 12.0f, 20, sf::Color::White);

    btnPlayAgain.setPosition({tableLeft + tableWidth - 220.0f, buttonsTop});
    renderWindow.draw(btnPlayAgain);
    drawCellText("Zagraj ponownie", tableLeft + tableWidth - 195.0f, buttonsTop + 12.0f, 20, sf::Color::White);
}