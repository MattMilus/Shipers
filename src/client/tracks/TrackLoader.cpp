//
// Created by Wiktor on 27.05.2026.
//

#include "TrackLoader.h"

#include "tracks/Track1.h"
#include <fstream>
#include <sstream>
#include <string>

static bool tryLoadFromConfig(GameManager* gameManager) {
    std::ifstream in("config.txt");
    if (!in.is_open()) return false;

    std::string line;
    enum class Section { None, Track, Barrier, Finish, Spawns, Coins } section = Section::None;
    std::vector<sf::Vector2f> points;
    float currentWidth = 150.0f;
    std::vector<sf::Vector2f> coinPositions;

    auto flushPointsAsTrack = [&]() {
        if (!points.empty()) {
            gameManager->generateTrack(points, currentWidth);
            points.clear();
        }
    };

    auto flushPointsAsBarrier = [&]() {
        if (!points.empty()) {
            gameManager->generateBarrier(points);
            points.clear();
        }
    };

    while (std::getline(in, line)) {
        // trim
        std::string s = line;
        auto start = s.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue;
        s = s.substr(start);
        if (s.empty() || s[0] == '#') continue;

        if (s.rfind("TRACK", 0) == 0) {
            flushPointsAsBarrier();
            flushPointsAsTrack();
            section = Section::Track;
            // parse width if provided: TRACK <width>
            std::istringstream iss(s);
            std::string tok; iss >> tok; iss >> currentWidth;
            continue;
        }

        if (s.rfind("BARRIER", 0) == 0) {
            flushPointsAsTrack();
            flushPointsAsBarrier();
            section = Section::Barrier;
            continue;
        }

        if (s.rfind("FINISH", 0) == 0) {
            flushPointsAsTrack();
            flushPointsAsBarrier();
            section = Section::Finish;
            std::istringstream iss(s);
            std::string tok; iss >> tok; float x,y; if (iss >> x >> y) gameManager->addFinish({x,y});
            section = Section::None;
            continue;
        }

        if (s.rfind("SPAWNS", 0) == 0) {
            flushPointsAsTrack();
            flushPointsAsBarrier();
            section = Section::Spawns;
            points.clear();
            continue;
        }

        if (s.rfind("COINS", 0) == 0) {
            flushPointsAsTrack();
            flushPointsAsBarrier();
            section = Section::Coins;
            continue;
        }

        if (s == "END") {
            if (section == Section::Track) flushPointsAsTrack();
            else if (section == Section::Barrier) flushPointsAsBarrier();
            else if (section == Section::Spawns) {
                if (!points.empty()) {
                    gameManager->setSpawnPoints(points);
                    points.clear();
                }
            }
            section = Section::None;
            continue;
        }

        // parse coordinates
        if (section == Section::Track || section == Section::Barrier || section == Section::Spawns) {
            float x,y; std::istringstream iss(s); if (iss >> x >> y) points.emplace_back(x,y);
        } else if (section == Section::Coins) {
            float x,y; std::istringstream iss(s); if (iss >> x >> y) coinPositions.emplace_back(x,y);
        }
    }

    // flush remaining
    if (!points.empty()) flushPointsAsTrack();
    if (!coinPositions.empty()) {
        gameManager->setCoinsFromConfig(coinPositions);
    }

    in.close();
    return true;
}

void TrackLoader::loadTrack(Tracks track, GameManager* gameManager) {
    // try config first
    if (tryLoadFromConfig(gameManager)) return;

    switch (track) {
        case Track1: {
            Track1::loadTrack(gameManager);
            break;
        }

        // Sadly we have only one course :(
        default: {
            Track1::loadTrack(gameManager);
            break;
        }

    }
}
