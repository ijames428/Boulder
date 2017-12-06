#ifndef DOOR_H
#define DOOR_H

using namespace std;
#include <SFML/Graphics.hpp>
#include "PlatformerLibrary\Box2D\Box2D.h"
#include "PlatformerLibrary\Box2DRigidBody.h"

class Door : public Box2DRigidBody {
	//b2BodyDef bodyDef;
	//b2Body* body;
	//b2PolygonShape boxShape;
	//b2FixtureDef boxFixtureDef;
	//b2Fixture* boxFixture;
	std::vector<string> names_of_activating_objects;
	bool opening;
	bool closing;
	int percent_open;
	float original_width;
	float original_height;
	float original_y;
public:
	Door(sf::RenderWindow *window, sf::Vector2f position = sf::Vector2f(0.0f, 0.0f), sf::Vector2f dimensions = sf::Vector2f(0.0f, 0.0f), bool subject_to_gravity = true);
	void TryToActivate(string activating_objects_name);
	virtual void Update(sf::Int64 curr_frame, sf::Int64 delta_time);
	void AddActivator(string activator_name);
};

#endif