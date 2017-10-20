#pragma once

#ifndef BOX2D_RIGIDBODY_H
#define BOX2D_RIGIDBODY_H

using namespace std;

#include <SFML/Graphics.hpp>
#include "Box2D\Common\b2Draw.h"
#include "Drawable.h"

class Box2DRigidBody : public Drawable {
protected:
	sf::Int64 current_frame;
private:
	int entity_type;
	sf::Vector2f current_position;
	sf::Vector2f current_dimensions;
	bool in_the_air;
	int id = rand();
	bool facing_right;
	bool facing_right_when_hit;
	bool lock_facing_direction_when_hit;
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
	Box2DRigidBody(sf::RenderWindow *window, sf::Vector2f position, sf::Vector2f dimensions, bool subject_to_gravity = false, bool subject_to_collision = true);
	virtual void Update(sf::Int64 curr_time, sf::Int64 delta_time);
	static float GetDistanceBetweenTwoPoints(sf::Vector2f point_a, sf::Vector2f point_b);
	void SetFacingRight(bool new_facing_right);
	void SetInTheAir(bool in_the_air_now);
	bool IsInTheAir();
};

#endif