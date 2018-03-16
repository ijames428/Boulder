#pragma once

using namespace std;
#include "stdafx.h"
#include <iostream>
#include "Door.h"
#include "SmashWorld.h"

Door::Door(sf::RenderWindow *window, string name, sf::Vector2f position, sf::Vector2f dimensions, bool subject_to_gravity) : Box2DRigidBody(window, position, dimensions, subject_to_gravity) {
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

	Name = name;
}

void Door::SetLocked(bool locked) {
	Locked = locked;
}

void Door::OpenDoor() {
	opening = true;
	closing = false;
}

void Door::CloseDoor() {
	closing = true;
	opening = false;
}

void Door::TryToActivate(string activating_objects_name, bool to_open) {
	if (!Locked) {
		if (std::find(names_of_activating_objects.begin(), names_of_activating_objects.end(), activating_objects_name) != names_of_activating_objects.end() ||
			activating_objects_name == Name) {
			/* v does contain x */
			if (percent_open == 100 && !to_open) {
				closing = true;
			}
			else if (percent_open == 0 && to_open) {
				opening = true;
			}
		}
		else {
			/* v does not contain x */
		}
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

void Door::ApplyObjectDataToSaveData(Json::Value& save_data) {
	//save_data["Name"] = name;
	save_data["percent_open"] = percent_open;
	save_data["opening"] = opening;
	save_data["closing"] = closing;
}

void Door::ApplySaveDataToObjectData(Json::Value& save_data) {
	percent_open = save_data["percent_open"].asInt();
	opening = save_data["opening"].asBool();
	closing = save_data["closing"].asBool();

	body->SetTransform(b2Vec2(body->GetPosition().x, original_y - (original_height * (percent_open * 0.01f) * 2.0f)), 0.0f);
}

bool Door::IfShouldUpdate(sf::Vector2f player_screen_pos, sf::Vector2f viewport_dimensions) {
	sf::Vector2f body_screen_pos = GetScreenPosition();

	if (abs(player_screen_pos.x - body_screen_pos.x) > viewport_dimensions.x * 1.25f) {
		return false;
	}

	if (abs(player_screen_pos.y - body_screen_pos.y) > viewport_dimensions.y * 1.25f) {
		return false;
	}

	return true;
}