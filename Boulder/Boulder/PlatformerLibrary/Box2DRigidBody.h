#pragma once

#ifndef BOX2D_RIGIDBODY_H
#define BOX2D_RIGIDBODY_H

using namespace std;

#include <SFML/Graphics.hpp>
#include "Box2D\Common\b2Draw.h"
#include "Drawable.h"
#include "Box2D\Box2D.h"
#include "..\GameLibrary\Json\json.h"

class Box2DRigidBody {
protected:
	sf::Int64 current_frame;
	b2BodyDef bodyDef;
	b2PolygonShape polygon;
	b2FixtureDef fixtureDef;
	b2Vec2 points[3];
	b2Body* body;
	b2Fixture *fixture;
private:
	int entity_type;
	sf::Vector2f current_position;
	sf::Vector2f current_dimensions;
	bool in_the_air;
	int id = rand();
	bool facing_right;
	bool facing_right_when_hit;
	bool lock_facing_direction_when_hit;
	bool isMovingPlatform;
	bool isElevator;
	bool isElevatorStopped;
	sf::Vector2f velocity;
	sf::Vector2f startingPosition;
	sf::Vector2f scheduledVelocity;
	b2Vec2 stoppedPosition;
	sf::Vector2f previousMovingVelocity;
	bool passThrough;
public:
	bool IsFacingRight();
	bool WasFacingRightWhenHit();
	void SetFacingRightWhenHit(bool facing_right_when_hit);
	sf::Vector2f GetCurrentPosition();
	sf::Vector2f GetCurrentDimensions();
	int GetEntityType();
	void SetEntityType(int new_entity_type);
	int GetID();
	void LockFacingDirection();
	void UnlockFacingDirection();
	bool IsFacingDirectionLocked();
	Box2DRigidBody(sf::RenderWindow *window, bool subject_to_gravity = false, bool subject_to_collision = true);
	Box2DRigidBody(sf::RenderWindow *window, string name, vector<string> json_points, bool subject_to_gravity = false, bool subject_to_collision = true);
	Box2DRigidBody(sf::RenderWindow *window, string name, sf::Vector2f position, sf::Vector2f dimensions, bool subject_to_gravity = false, bool subject_to_collision = true);
	virtual void Update(sf::Int64 curr_time, sf::Int64 delta_time);
	static float GetDistanceBetweenTwoPoints(sf::Vector2f point_a, sf::Vector2f point_b);
	void SetFacingRight(bool new_facing_right);
	void SetInTheAir(bool in_the_air_now);
	bool IsInTheAir();
	void ConvertToMovingPlatform(float vel_x, float vel_y);
	void StartMovingElevator(float vel_x, float vel_y);
	void ChangeDirection(float vel_x, float vel_y);
	void ChangeDirection(sf::Vector2f vel);
	sf::Vector2f GetVelocity();
	void HandleCollision();
	bool IfShouldUpdate(sf::Vector2f player_screen_pos, sf::Vector2f viewport_dimensions);
	string Name;
	void SetPassThrough(bool pass_through);
	bool IsPassThroughable();
	bool IsElevator() {
		return isElevator;
	}
	bool IsElevatorStopped() {
		return isElevatorStopped;
	}
	sf::Vector2f GetScreenPosition() {
		return sf::Vector2f(body->GetPosition().x * 40.0f, body->GetPosition().y * 40.0f);
	}

	b2Body* GetBody() {
		return body;
	}

	virtual void ApplyObjectDataToSaveData(Json::Value& save_data);
	virtual void ApplySaveDataToObjectData(Json::Value& save_data);
};

#endif