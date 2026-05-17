#ifndef TRACK_H
#define TRACK_H

#pragma once
#include <SFML/System/Vector2.hpp>
#include <vector>

struct Bouy {
	Bouy(sf::Vector2f pos) : position(pos) {}
	sf::Vector2f position;
	float radius = 25.0f;
};

class Track {
	std::vector<Bouy> bouys; 

public:
	Track() = default;
	void generateTrack(std::vector<sf::Vector2f> controlPoints);
	const std::vector<Bouy>& getBouys() const { return bouys; }
};

#endif //TRACK_H