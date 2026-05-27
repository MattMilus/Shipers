#ifndef TRACK_H
#define TRACK_H

#pragma once
#include <SFML/System/Vector2.hpp>
#include <vector>

constexpr auto DEFAULT_FINISH_RADIUS = 100.0f;

struct Buoy {
	explicit Buoy(sf::Vector2f pos) : position(pos) {}
	sf::Vector2f position;
	float radius = 25.0f;
};

class Track {
	std::vector<Buoy> buoys;
	Buoy finishBuoy = Buoy(sf::Vector2f{0, 0});
	float finishRadius = DEFAULT_FINISH_RADIUS;

	std::vector<sf::Vector2f> spawnPoints;

public:
	Track() = default;

	void generateTrack(std::vector<sf::Vector2f> controlPoints, float trackWidth = 150.0f);
	void generateBarrier(std::vector<sf::Vector2f> controlPoints);

	void setFinish(sf::Vector2f pos);
	void setSpawnPoints(const std::vector<sf::Vector2f>& spawns);

	[[nodiscard]] const std::vector<Buoy>& getBuoys() const { return buoys; }
	[[nodiscard]] Buoy getFinishBuoy() const { return finishBuoy; }
	[[nodiscard]] float getFinishRadius() const { return finishRadius; }
	[[nodiscard]] const sf::Vector2f& getFinishPos() const { return finishBuoy.position; }

	[[nodiscard]] const std::vector<sf::Vector2f>& getSpawnPoints() const { return spawnPoints; } // Nowy getter

	[[nodiscard]] bool isBoatFinished(sf::Vector2f pos) const;
};

#endif //TRACK_H