#ifndef TRACK_H
#define TRACK_H

#pragma once
#include <SFML/System/Vector2.hpp>
#include <vector>

struct Buoy {
	Buoy(sf::Vector2f pos) : position(pos) {}
	sf::Vector2f position;
	float radius = 25.0f;
};

class Track {
	std::vector<Buoy> buoys;

public:
	Track() = default;
	void generateTrack(std::vector<sf::Vector2f> controlPoints);
	const std::vector<Buoy>& getBouys() const { return buoys; }
};

#endif //TRACK_H