#pragma once

using namespace std;
#include "stdafx.h"
#include <iostream>
#include "Platform.h"
#include "..\Constants.h"

Platform::Platform(sf::RenderWindow *window, sf::Vector2f position, sf::Vector2f dimensions, bool subject_to_gravity) : RigidBody(position, dimensions, subject_to_gravity) {
	SetEntityType(Constants::ENTITY_TYPE_PLATFORM);
	render_window = window;

	sf::RectangleShape shape(dimensions);
	shape.setFillColor(sf::Color::Green);
	shape.setPosition(position);

	ExcludeFromCollision(GetEntityType());

	rectangle_shape = shape;
}

void Platform::Draw(sf::Vector2f camera_position) {
	rectangle_shape.setPosition(sf::Vector2f(GetCurrentPosition().x - camera_position.x, GetCurrentPosition().y - camera_position.y));
	render_window->draw(rectangle_shape);
}