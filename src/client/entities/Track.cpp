#include "Track.h"
#include <cmath>

namespace {
    sf::Vector2f normalize(const sf::Vector2f& source) {
        float length = std::hypot(source.x, source.y);
        if (length != 0) return { source.x / length, source.y / length };
        return source;
    }
}

// Bezier curve++
sf::Vector2f getBSplinePoint(const sf::Vector2f& p0,
                             const sf::Vector2f& p1,
                             const sf::Vector2f& p2,
                             const sf::Vector2f& p3,
                             float t) {
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

void Track::generateBarrier(std::vector<sf::Vector2f> controlPoints) {
    if (controlPoints.empty()) return;

    float separationDistance = 50.f;
    int segmentsPerCurve = 20;

    buoys.reserve(buoys.size() + (controlPoints.size() * segmentsPerCurve) + 50);

    std::vector<sf::Vector2f> paddedPoints;
    paddedPoints.push_back(controlPoints.front());
    paddedPoints.push_back(controlPoints.front());
    for (const auto& p : controlPoints) {
        paddedPoints.push_back(p);
    }
    paddedPoints.push_back(controlPoints.back());
    paddedPoints.push_back(controlPoints.back());

    for (size_t i = 0; i < paddedPoints.size() - 3; ++i) {
        for (int j = 0; j <= segmentsPerCurve; ++j) {
            if (j == 0 && i > 0) continue;

            sf::Vector2f nextPos = getBSplinePoint(
                paddedPoints[i], paddedPoints[i + 1],
                paddedPoints[i + 2], paddedPoints[i + 3],
                static_cast<float>(j) / segmentsPerCurve
            );

          if (!buoys.empty()) {
             sf::Vector2f lastPos = buoys.back().position;
             if (std::hypot(nextPos.x - lastPos.x, nextPos.y - lastPos.y) < separationDistance) {
                continue;
             }
          }

            buoys.emplace_back(nextPos);
        }
    }
}

void Track::generateTrack(std::vector<sf::Vector2f> controlPoints, float trackWidth) {
    if (controlPoints.size() < 2) return;

    float separationDistance = 50.f;
    int segmentsPerCurve = 20;

    buoys.reserve(buoys.size() + (controlPoints.size() * segmentsPerCurve * 2) + 100);

    std::vector<sf::Vector2f> paddedPoints;
    paddedPoints.push_back(controlPoints.front());
    paddedPoints.push_back(controlPoints.front());
    for (const auto& p : controlPoints) paddedPoints.push_back(p);
    paddedPoints.push_back(controlPoints.back());
    paddedPoints.push_back(controlPoints.back());

    std::vector<sf::Vector2f> splinePoints;
    for (size_t i = 0; i < paddedPoints.size() - 3; ++i) {
        for (int j = 0; j <= segmentsPerCurve; ++j) {
            if (j == 0 && i > 0) continue;
            splinePoints.push_back(getBSplinePoint(
                paddedPoints[i], paddedPoints[i + 1],
                paddedPoints[i + 2], paddedPoints[i + 3],
                static_cast<float>(j) / segmentsPerCurve
            ));
        }
    }

    if (splinePoints.size() < 2) return;

    sf::Vector2f lastLeftPos(-9999.f, -9999.f);
    sf::Vector2f lastRightPos(-9999.f, -9999.f);

    for (size_t i = 0; i < splinePoints.size(); ++i) {
        sf::Vector2f dir;
        if (i < splinePoints.size() - 1) dir = splinePoints[i + 1] - splinePoints[i];
        else dir = splinePoints[i] - splinePoints[i - 1];

        sf::Vector2f normal = normalize({ -dir.y, dir.x });

        sf::Vector2f leftPos = splinePoints[i] + normal * trackWidth;
        sf::Vector2f rightPos = splinePoints[i] - normal * trackWidth;

        // Banda Lewa
        if (i == 0 || std::hypot(leftPos.x - lastLeftPos.x, leftPos.y - lastLeftPos.y) >= separationDistance) {
            buoys.emplace_back(leftPos);
            lastLeftPos = leftPos;
        }

        // Banda Prawa
        if (i == 0 || std::hypot(rightPos.x - lastRightPos.x, rightPos.y - lastRightPos.y) >= separationDistance) {
            buoys.emplace_back(rightPos);
            lastRightPos = rightPos;
        }
    }
}

void Track::setFinish(sf::Vector2f pos) {
    this->finishBuoy = Buoy(pos);
}

void Track::setSpawnPoints(const std::vector<sf::Vector2f>& spawns) {
    this->spawnPoints = spawns;
}

bool Track::isBoatFinished(sf::Vector2f pos) const {
    return std::hypot(this->getFinishPos().x - pos.x, this->getFinishPos().y - pos.y) < this->finishRadius;
}