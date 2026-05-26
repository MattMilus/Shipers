#include "Track.h"
#include <cmath>

//Bezier curve++
sf::Vector2f getBSplinePoint(const sf::Vector2f& p0, 
                             const sf::Vector2f& p1, 
                             const sf::Vector2f& p2, 
                             const sf::Vector2f& p3, 
                             float t) {
    float it = 1.0 - t;
    float t2 = t * t;
    float t3 = t2 * t;

    float b0 = (it * it * it) / 6.0;
    float b1 = (3.0 * t3 - 6.0 * t2 + 4.0) / 6.0;
    float b2 = (-3.0 * t3 + 3.0 * t2 + 3.0 * t + 1.0) / 6.0;
    float b3 = t3 / 6.0;

    return {
        b0 * p0.x + b1 * p1.x + b2 * p2.x + b3 * p3.x,
        b0 * p0.y + b1 * p1.y + b2 * p2.y + b3 * p3.y
    };
}

void Track::generateTrack(std::vector<sf::Vector2f> controlPoints) {
    //buoys.clear(); // delete for multiple bounds

    float separationDistance = 50.f;

    std::vector<sf::Vector2f> paddedPoints;
    paddedPoints.push_back(controlPoints.front());
    paddedPoints.push_back(controlPoints.front());
    for (const auto& p : controlPoints) {
        paddedPoints.push_back(p);
    }
    paddedPoints.push_back(controlPoints.back());
    paddedPoints.push_back(controlPoints.back());

	int segmentsPerCurve = 20;
    for (size_t i = 0; i < paddedPoints.size() - 3; ++i) {
        for (int j = 0; j <= segmentsPerCurve; ++j) {
            // Prevent duplicate points at segment junctions
            if (j == 0 && i > 0) continue;

            sf::Vector2f nextPos = getBSplinePoint(
                paddedPoints[i],
                paddedPoints[i + 1],
                paddedPoints[i + 2],
                paddedPoints[i + 3],
                static_cast<double>(j) / segmentsPerCurve
            );

			// Prevent bouys from being too close to each other
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