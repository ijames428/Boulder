#pragma once

using namespace std;
#include "stdafx.h"
#include <iostream>
#include "Checkpoint.h"
#include "..\GameLibrary\Singleton.h"
#include "../Constants.h"

Checkpoint::Checkpoint(sf::RenderWindow *window, sf::Vector2f position, sf::Vector2f dimensions, bool subject_to_gravity) : RigidBody(position, dimensions, subject_to_gravity) {
	SetEntityType(Constants::ENTITY_TYPE_CHECKPOINT);
	DisableCollision();
	render_window = window;

	sf::RectangleShape shape(dimensions);
	shape.setFillColor(sf::Color::Yellow);
	shape.setFillColor(sf::Color(shape.getFillColor().r, shape.getFillColor().g, shape.getFillColor().b, 127));
	shape.setPosition(position);

	rectangle_shape = shape;
}

void Checkpoint::Draw(sf::Vector2f camera_position) {
#ifdef _DEBUG
	rectangle_shape.setPosition(sf::Vector2f(GetCurrentPosition().x - camera_position.x, GetCurrentPosition().y - camera_position.y));
	render_window->draw(rectangle_shape);
#endif
}

void Checkpoint::UpdateCheckPoint() {
	std::vector<RigidBody*> hit_objects = GetCollidersRigidBodyIsCollidingWith();

	for (int i = 0; i < (int)hit_objects.size(); i++) {
		if (hit_objects[i]->GetEntityType() == Constants::ENTITY_TYPE_PLAYER_CHARACTER) {
			//TODO: Address setting of CurrentCheckpoint.
			//Singleton<World>::Get()->SetCurrentCheckPoint(this);
			break;
		}
	}
}