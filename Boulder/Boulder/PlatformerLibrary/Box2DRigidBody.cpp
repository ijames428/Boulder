#pragma once

#include "stdafx.h"
#include <iostream>
#include "Box2DRigidBody.h"
#include "GridHandler.h"
#include "..\GameLibrary\Singleton.h"
#include "..\Constants.h"
#include "..\SmashWorld.h"

Box2DRigidBody::Box2DRigidBody(sf::RenderWindow *window, bool subject_to_gravity, bool subject_to_collision) {
	entity_type = Constants::ENTITY_TYPE_RIGID_BODY;
	in_the_air = true;
	facing_right = true;
}

Box2DRigidBody::Box2DRigidBody(sf::RenderWindow *window, vector<string> json_points, bool subject_to_gravity, bool subject_to_collision) {
	float x = 0.0f;
	float y = 0.0f;
	float width = 0.0f;
	float height = 0.0f;
	std::string::size_type sz;

	float scalingRatio = 10.0f;

	std::vector<string> vect0;
	std::vector<string> vect1;
	std::vector<string> vect2;

	string str = json_points[0];
	size_t commaLocation = str.find(',', 0);
	vect0.push_back(str.substr(0, commaLocation));
	vect0.push_back(str.substr(commaLocation + 1));

	str = json_points[1];
	commaLocation = str.find(',', 0);
	vect1.push_back(str.substr(0, commaLocation));
	vect1.push_back(str.substr(commaLocation + 1));

	str = json_points[2];
	commaLocation = str.find(',', 0);
	vect2.push_back(str.substr(0, commaLocation));
	vect2.push_back(str.substr(commaLocation + 1));

	points[0] = b2Vec2(std::stof(vect0[0], &sz) / scalingRatio, std::stof(vect0[1], &sz) / scalingRatio);
	points[1] = b2Vec2(std::stof(vect1[0], &sz) / scalingRatio, std::stof(vect1[1], &sz) / scalingRatio);
	points[2] = b2Vec2(std::stof(vect2[0], &sz) / scalingRatio, std::stof(vect2[1], &sz) / scalingRatio); 
	
	bodyDef.type = b2_staticBody;

	body = Singleton<SmashWorld>::Get()->GetB2World()->CreateBody(&bodyDef);
	polygon.Set(points, 3);

	fixtureDef.shape = &polygon;
	fixtureDef.density = 0.0f;
	fixtureDef.friction = 0.9f;
	fixtureDef.m_color = new b2Color(1.0f, 1.0f, 0.0f, 1.0f);
	fixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->PLATFORM;

	body->CreateFixture(&fixtureDef);

	entity_type = Constants::ENTITY_TYPE_RIGID_BODY;
	in_the_air = true;
	facing_right = true;
}

Box2DRigidBody::Box2DRigidBody(sf::RenderWindow *window, sf::Vector2f position, sf::Vector2f dimensions, bool subject_to_gravity, bool subject_to_collision) {
	bodyDef.position.Set(position.x, position.y);
	bodyDef.type = b2_staticBody;
	body = Singleton<SmashWorld>::Get()->GetB2World()->CreateBody(&bodyDef);
	polygon.SetAsBox(dimensions.x, dimensions.y);

	fixtureDef.shape = &polygon;
	fixtureDef.density = 0.0f;
	fixtureDef.friction = 0.9f;
	fixtureDef.m_color = new b2Color(1.0f, 1.0f, 0.0f, 1.0f);
	fixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->PLATFORM;

	fixture = body->CreateFixture(&fixtureDef);

	entity_type = Constants::ENTITY_TYPE_RIGID_BODY;
	in_the_air = true;
	facing_right = true;
}

void Box2DRigidBody::Update(sf::Int64 curr_frame, sf::Int64 delta_time) {
	current_frame = curr_frame;
}

float Box2DRigidBody::GetDistanceBetweenTwoPoints(sf::Vector2f point_a, sf::Vector2f point_b) {
	return sqrt((point_a.x - point_b.x) * (point_a.x - point_b.x) + (point_a.y - point_b.y) * (point_a.y - point_b.y));
}

bool Box2DRigidBody::IsFacingRight() {
	return facing_right;
}

sf::Vector2f Box2DRigidBody::GetCurrentPosition() {
	return current_position;
}

sf::Vector2f Box2DRigidBody::GetCurrentDimensions() {
	return current_dimensions;
}

int Box2DRigidBody::GetEntityType() {
	return entity_type;
}

void Box2DRigidBody::SetEntityType(int new_entity_type) {
	entity_type = new_entity_type;
}

void Box2DRigidBody::LockFacingDirection() {
	lock_facing_direction_when_hit = true;
}

void Box2DRigidBody::UnlockFacingDirection() {
	lock_facing_direction_when_hit = false;
}

bool Box2DRigidBody::IsFacingDirectionLocked() {
	return lock_facing_direction_when_hit;
}

bool Box2DRigidBody::WasFacingRightWhenHit() {
	return facing_right_when_hit;
}

void Box2DRigidBody::SetFacingRightWhenHit(bool facing_right_when_hit) {
	facing_right = facing_right_when_hit;
}

void Box2DRigidBody::SetFacingRight(bool new_facing_right) {
	facing_right = new_facing_right;
}

int Box2DRigidBody::GetID() {
	return id;
}

void Box2DRigidBody::SetInTheAir(bool in_the_air_now) {
	in_the_air = in_the_air_now;
}

bool Box2DRigidBody::IsInTheAir() {
	return in_the_air;
}

void Box2DRigidBody::ApplyObjectDataToSaveData(Json::Value& save_data) {
}

void Box2DRigidBody::ApplySaveDataToObjectData(Json::Value& save_data) {
}