#ifndef TRIGGER_H
#define TRIGGER_H

using namespace std;
#include <SFML/Graphics.hpp>
#include "PlatformerLibrary\Box2D\Box2D.h"
#include "PlatformerLibrary\Box2DRigidBody.h"

class Trigger : public Box2DRigidBody {
private:
	string Name;
	std::vector<string> activities;
public:
	Trigger(string name, sf::RenderWindow *window, sf::Vector2f position = sf::Vector2f(0.0f, 0.0f), sf::Vector2f dimensions = sf::Vector2f(0.0f, 0.0f), bool subject_to_gravity = true);
	void Triggered();
	void AddActivaty(string activity);
	sf::Vector2f viewport_position;
	sf::Vector2f viewport_dimensions;
};

#endif