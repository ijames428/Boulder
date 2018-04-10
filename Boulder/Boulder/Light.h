#ifndef LIGHT_H
#define LIGHT_H

using namespace std;
#include <SFML/Graphics.hpp>

class Light {
public:
	sf::Vector2f position;
	float red;
	float green;
	float blue;
	Light(sf::Vector2f pos, float r, float g, float b);
};

#endif