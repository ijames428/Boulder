#pragma once

using namespace std;
#include "stdafx.h"
#include <iostream>
#include "Door.h"
#include "SmashWorld.h"

Door::Door(sf::RenderWindow *window, sf::Vector2f position, sf::Vector2f dimensions, bool subject_to_gravity) : Box2DRigidBody(window, position, dimensions, subject_to_gravity) {
	opening = false;
	closing = false;
	percent_open = 0;
	original_width = dimensions.x;
	original_height = dimensions.y;
	fixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->DOOR;
	fixture->SetFilterData(fixtureDef.filter);

	names_of_activating_objects = std::vector<string>();

	body->SetUserData(this);

	original_y = body->GetPosition().y;
	//bodyDef.type = b2_dynamicBody;
	//bodyDef.position.Set(position.x, position.y);
	//body = Singleton<SmashWorld>::Get()->GetB2World()->CreateBody(&bodyDef);
	//
	//boxShape.SetAsBox(0.4f, 1.2f);
	//boxFixtureDef.shape = &boxShape;
	//boxFixtureDef.density = 1.0f;
	//boxFixtureDef.friction = 0.3f;
	//boxFixtureDef.m_color = new b2Color(0.0f, 1.0f, 0.0f, 1.0f);
	//boxFixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->DOOR;
	//boxFixture = body->CreateFixture(&boxFixtureDef);
}

void Door::TryToActivate(string activating_objects_name) {
	if (std::find(names_of_activating_objects.begin(), names_of_activating_objects.end(), activating_objects_name) != names_of_activating_objects.end()) {
		/* v does contain x */
		if (percent_open == 100) {
			closing = true;
		} else if (percent_open == 0) {
			opening = true;
		}
	} else {
		/* v does not contain x */
	}
}

void Door::Update(sf::Int64 curr_frame, sf::Int64 delta_time) {
	if (opening) {
		if (percent_open < 100) {
			percent_open += 2;
		} else {
			percent_open = 100;
			opening = false;
		}

		body->SetTransform(b2Vec2(body->GetPosition().x, original_y - (original_height * (percent_open * 0.01f) * 2.0f)), 0.0f);
		//polygon.m_vertices[2].Set(polygon.m_vertices[2].x, (original_height * (percent_open * 0.01f)));
		//polygon.m_vertices[3].Set(polygon.m_vertices[3].x, (original_height * (percent_open * 0.01f)));
		//polygon.SetAsBox(original_width, 0.0f);
	}

	if (closing) {
		if (percent_open > 0) {
			percent_open -= 2;
		} else {
			percent_open = 0;
			closing = false;
		}

		body->SetTransform(b2Vec2(body->GetPosition().x, original_y - (original_height * (percent_open * 0.01f) * 2.0f)), 0.0f);
		//polygon.m_vertices[2].Set(polygon.m_vertices[2].x, (original_height * (percent_open * 0.01f)));
		//polygon.m_vertices[3].Set(polygon.m_vertices[3].x, (original_height * (percent_open * 0.01f)));
		//polygon.SetAsBox(original_width, original_height);
	}
}

void Door::AddActivator(string activator_name) {
	if (std::find(names_of_activating_objects.begin(), names_of_activating_objects.end(), activator_name) == names_of_activating_objects.end()) {
		names_of_activating_objects.push_back(activator_name);
	}
}