#ifndef CAMERA_H
#define CAMERA_H

using namespace std;
#include <SFML/Graphics.hpp>

class Camera {
public:
	sf::Vector2f viewport_position;
	sf::Vector2f viewport_dimensions;
	Camera(sf::Vector2f position, sf::Vector2f dimensions);
};

#endif