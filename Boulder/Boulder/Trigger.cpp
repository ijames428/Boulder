#pragma once

using namespace std;
#include "stdafx.h"
#include <iostream>
#include "Trigger.h"
#include "GameLibrary\Singleton.h"
#include "SmashWorld.h"

Trigger::Trigger(string name, sf::RenderWindow *window, sf::Vector2f position, sf::Vector2f dimensions, bool subject_to_gravity) : Box2DRigidBody(window, subject_to_gravity) {
	Name = name;
	viewport_position = position;
	viewport_dimensions = dimensions;

	bodyDef.position.Set(position.x, position.y);
	body = Singleton<SmashWorld>::Get()->GetB2World()->CreateBody(&bodyDef);
	polygon.SetAsBox(dimensions.x, dimensions.y);

	fixtureDef.shape = &polygon;
	fixtureDef.isSensor = true;
	fixtureDef.m_color = new b2Color(0.0f, 1.0f, 0.0f, 1.0f);
	fixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->TRIGGER;

	fixture = body->CreateFixture(&fixtureDef);

	body->SetUserData(this);
}

void Trigger::Triggered() {
	Singleton<SmashWorld>::Get()->ExecuteAction(activities[0]);
}

void Trigger::AddActivaty(string activity) {
	if (std::find(activities.begin(), activities.end(), activity) == activities.end()) {
		activities.push_back(activity);
	}
}