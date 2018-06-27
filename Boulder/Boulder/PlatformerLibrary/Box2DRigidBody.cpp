#pragma once

#include "stdafx.h"
#include <iostream>
#include "Box2DRigidBody.h"
#include "GridHandler.h"
#include "..\GameLibrary\Singleton.h"
#include "..\Constants.h"
#include "..\SmashWorld.h"

Box2DRigidBody::Box2DRigidBody(sf::RenderWindow *window, bool subject_to_gravity, bool subject_to_collision) {
	render_window = window;

	entity_type = Constants::ENTITY_TYPE_RIGID_BODY;
	in_the_air = true;
	facing_right = true;
	isMovingPlatform = false;
	isElevator = false;
	isElevatorStopped = true;
	previousMovingVelocity = sf::Vector2f(0.0f, 0.0f);
	TiedArtImageFileName = "";
	halfBodyHeight = 0.0f;

	Name = "";
}

Box2DRigidBody::Box2DRigidBody(sf::RenderWindow *window, string name, vector<string> json_points, bool subject_to_gravity, bool subject_to_collision) {
	render_window = window;

	Name = name;
	halfBodyHeight = 0.0f;

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
	body->SetGravityScale(0.0f);
	polygon.Set(points, 3);

	fixtureDef.shape = &polygon;
	fixtureDef.density = 0.0f;
	fixtureDef.friction = 1.0f;
	fixtureDef.m_color = new b2Color(1.0f, 1.0f, 0.0f, 1.0f);
	fixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->PLATFORM;

	body->CreateFixture(&fixtureDef);

	entity_type = Constants::ENTITY_TYPE_RIGID_BODY;
	in_the_air = true;
	facing_right = true;
	isMovingPlatform = false;
	isElevator = false;
	isElevatorStopped = true;
	previousMovingVelocity = sf::Vector2f(0.0f, 0.0f);
	TiedArtImageFileName = "";

	body->SetUserData(this);
}

Box2DRigidBody::Box2DRigidBody(sf::RenderWindow *window, string name, sf::Vector2f position, sf::Vector2f dimensions, bool subject_to_gravity, bool subject_to_collision) {
	render_window = window;

	bodyDef.position.Set(position.x, position.y);
	bodyDef.type = b2_staticBody;
	body = Singleton<SmashWorld>::Get()->GetB2World()->CreateBody(&bodyDef);
	body->SetGravityScale(0.0f);
	polygon.SetAsBox(dimensions.x, dimensions.y);
	halfBodyHeight = dimensions.y / 2.0f;

	Name = name;

	startingPosition = position;

	fixtureDef.shape = &polygon;
	fixtureDef.density = 0.0f;
	fixtureDef.friction = 1.0f;
	fixtureDef.m_color = new b2Color(1.0f, 1.0f, 0.0f, 1.0f);
	fixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->PLATFORM;

	fixture = body->CreateFixture(&fixtureDef);

	entity_type = Constants::ENTITY_TYPE_RIGID_BODY;
	in_the_air = true;
	facing_right = true;
	isMovingPlatform = false;
	isElevator = false;
	isElevatorStopped = true;
	previousMovingVelocity = sf::Vector2f(0.0f, 0.0f);
	TiedArtImageFileName = "";

	body->SetUserData(this);
}

Box2DRigidBody::Box2DRigidBody(sf::RenderWindow *window, string name, float x1, float y1, float x2, float y2, bool subject_to_gravity, bool subject_to_collision) {
	render_window = window;

	bodyDef.position.Set(x1, y1);
	bodyDef.type = b2_staticBody;
	body = Singleton<SmashWorld>::Get()->GetB2World()->CreateBody(&bodyDef);
	body->SetGravityScale(0.0f);

	float scalingRatio = 1.0f;
	edge.Set(b2Vec2(x1 / scalingRatio, y1 / scalingRatio), b2Vec2(x2 / scalingRatio, y2 / scalingRatio));

	Name = name;

	startingPosition = sf::Vector2f (x1, y1);

	fixtureDef.shape = &edge;
	fixtureDef.density = 0.0f;
	fixtureDef.friction = 1.0f;
	fixtureDef.m_color = new b2Color(0.0f, 0.0f, 1.0f, 1.0f);
	fixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->PLATFORM;

	fixture = body->CreateFixture(&fixtureDef);

	entity_type = Constants::ENTITY_TYPE_RIGID_BODY;
	in_the_air = true;
	facing_right = true;
	isMovingPlatform = false;
	isElevator = false;
	isElevatorStopped = true;
	previousMovingVelocity = sf::Vector2f(0.0f, 0.0f);
	TiedArtImageFileName = "";

	body->SetUserData(this);
}

void Box2DRigidBody::Update(sf::Int64 curr_frame, sf::Int64 delta_time) {
	current_frame = curr_frame;

	if (!isElevator && (scheduledVelocity.x != 0.0f || scheduledVelocity.y != 0.0f)) {
		isElevator = true;

		body->SetType(b2BodyType::b2_dynamicBody);
		fixtureDef.filter.maskBits = Singleton<SmashWorld>::Get()->PLATFORM;
		body->SetUserData(this);
	}

	if (isElevator) {
		if (scheduledVelocity.x != 0.0f || scheduledVelocity.y != 0.0f) {
			isElevatorStopped = false;
			velocity = sf::Vector2f(scheduledVelocity.x, scheduledVelocity.y);
			body->SetLinearVelocity(b2Vec2(velocity.x, velocity.y));
		}

		body->SetLinearVelocity(b2Vec2(velocity.x, velocity.y));

		if (isElevatorStopped) {
			body->SetTransform(stoppedPosition, 0.0f);
		} else {
			if (velocity.x == 0.0f) {
				body->SetTransform(b2Vec2(startingPosition.x, body->GetTransform().p.y), 0.0f);
			}
			else if (velocity.y == 0.0f) {
				body->SetTransform(b2Vec2(body->GetTransform().p.x, startingPosition.y), 0.0f);
			}
		}
	}

	if (isMovingPlatform) {
		body->SetLinearVelocity(b2Vec2(velocity.x, velocity.y));

		if (velocity.x == 0.0f) {
			body->SetTransform(b2Vec2(startingPosition.x, body->GetTransform().p.y), 0.0f);
		} else if (velocity.y == 0.0f) {
			body->SetTransform(b2Vec2(body->GetTransform().p.x, startingPosition.y), 0.0f);
		}
	}
}

void Box2DRigidBody::Draw(sf::Vector2f camera_position) {
	if (TiedArtImageSprite != nullptr) {
		bodyPosition = body->GetPosition();

		//TiedArtImageSprite->setPosition(sf::Vector2f((position.x - camera_position.x) * 40.0f + 0.0f, (position.y - camera_position.y) * 40.0f + 0.0f));
		TiedArtImageSprite->setPosition(sf::Vector2f((bodyPosition.x - (TiedArtImageTextureSize.x / 2.0f * TiedArtImageSprite->getScale().x / 40.0f) - camera_position.x) * 40.0f,
													 (bodyPosition.y - (TiedArtImageTextureSize.y / 2.0f * TiedArtImageSprite->getScale().y / 40.0f) - camera_position.y) * 40.0f));

		render_window->draw(*TiedArtImageSprite);
	}
}

void Box2DRigidBody::LoadArtImage(string file_name) {
	TiedArtImageTexture = Singleton<AssetManager>().Get()->GetTexture(file_name);
	TiedArtImageTextureSize = TiedArtImageTexture->getSize();

	TiedArtImageSprite = new sf::Sprite(*TiedArtImageTexture);
	TiedArtImageSprite->setScale(0.666667f, 0.68f);
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

void Box2DRigidBody::SetPassThrough(bool pass_through) {
	passThrough = pass_through;
	//fixture->SetColor(0.8f, 0.8f, 0.0f, 1.0f);
}

bool Box2DRigidBody::IsPassThroughable() {
	return passThrough;
}

void Box2DRigidBody::ConvertToMovingPlatform(float vel_x, float vel_y) {
	isMovingPlatform = true;
	velocity = sf::Vector2f(vel_x, vel_y);
	body->SetLinearVelocity(b2Vec2(velocity.x, velocity.y));
	body->SetType(b2BodyType::b2_dynamicBody);
	fixtureDef.filter.maskBits = Singleton<SmashWorld>::Get()->PLATFORM;

	body->SetUserData(this);
}

void Box2DRigidBody::StartMovingElevator(float vel_x, float vel_y) {
	if (isElevatorStopped && 
		((previousMovingVelocity.x == 0.0f && previousMovingVelocity.y == 0.0f) ||
		(((previousMovingVelocity.x < 0.0f && vel_x > 0.0f) || (previousMovingVelocity.x > 0.0f && vel_x < 0.0f)) || 
								((previousMovingVelocity.y < 0.0f && vel_y > 0.0f) || (previousMovingVelocity.y > 0.0f && vel_y < 0.0f))))) {
		scheduledVelocity = sf::Vector2f(vel_x, vel_y);
		previousMovingVelocity = scheduledVelocity;
	}
}

void Box2DRigidBody::HandleCollision() {
	if (isElevator) {
		isElevatorStopped = true;

		stoppedPosition = body->GetTransform().p;

		scheduledVelocity = velocity = sf::Vector2f(0.0f, 0.0f);
		body->SetLinearVelocity(b2Vec2(velocity.x, velocity.y));
	} else if (isMovingPlatform) {
		velocity = sf::Vector2f(velocity.x * -1.0f, velocity.y * -1.0f);
		body->SetLinearVelocity(b2Vec2(velocity.x, velocity.y));
	} 
}

bool Box2DRigidBody::IfShouldUpdate(sf::Vector2f player_screen_pos, sf::Vector2f viewport_dimensions) {
	sf::Vector2f body_screen_pos = GetScreenPosition();

	if (abs(player_screen_pos.x - body_screen_pos.x) > viewport_dimensions.x * 1.25f) {
		return false;
	}

	if (abs(player_screen_pos.y - body_screen_pos.y) > viewport_dimensions.y * 1.25f) {
		return false;
	}

	return true;
}

void Box2DRigidBody::ChangeDirection(float vel_x, float vel_y) {
	velocity = sf::Vector2f(vel_x, vel_y);
}

void Box2DRigidBody::ChangeDirection(sf::Vector2f vel) {
	velocity = vel;
}

sf::Vector2f Box2DRigidBody::GetVelocity() {
	return velocity;
}