#ifndef END_OF_THE_GAME_H
#define END_OF_THE_GAME_H

using namespace std;
#include <SFML/Graphics.hpp>
#include "RigidBody.h"

class EndOfTheGame : public RigidBody {
public:
	sf::RenderWindow *render_window;
	sf::RectangleShape rectangle_shape;
	EndOfTheGame(sf::RenderWindow *window, sf::Vector2f position = sf::Vector2f(0.0f, 0.0f), sf::Vector2f dimensions = sf::Vector2f(0.0f, 0.0f), bool subject_to_gravity = true);
	void Draw(sf::Vector2f camera_position);
	void UpdateEndOfTheGame();
};

#endif