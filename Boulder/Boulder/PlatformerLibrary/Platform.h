#ifndef PLATFORM_H
#define PLATFORM_H

#include "..\GameLibrary\Singleton.h"
#include "RigidBody.h"

class Platform : public RigidBody {
public:
	sf::RectangleShape rectangle_shape;
	sf::RenderWindow *render_window;
	Platform(sf::RenderWindow *window, sf::Vector2f position, sf::Vector2f dimensions, bool subject_to_gravity = false);
	void Draw(sf::Vector2f camera_position);
};

#endif