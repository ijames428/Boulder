#pragma once

#ifndef RIGIDBODY_H
#define RIGIDBODY_H

using namespace std;

#include <SFML/Graphics.hpp>

class RigidBody {
protected:
	sf::Int64 current_time;
private:
	int entity_type;
	sf::Vector2f velocity;
	sf::Vector2f current_position;
	sf::Vector2f current_dimensions;
	sf::Vector2f future_position;
	sf::Vector2f future_dimensions;
	sf::Vector2f past_position;
	sf::Vector2f past_dimensions;
	sf::Vector2i grid_top_left;
	sf::Vector2i grid_bot_right;
	float gravity_acceleration;
	float terminal_velocity;
	float weight;
	bool gravity_enabled;
	bool collision_enabled;
	bool only_collide_with_platforms;
	bool in_the_air;
	int id = rand();
	bool facing_right;
	bool facing_right_when_hit;
	bool lock_facing_direction_when_hit;
	std::vector<int> entities_excluded_from_collision;
	bool AreTheRigidBodiesCollidingHorizontally(RigidBody* rb1, RigidBody* rb2, bool just_check_no_response = false);
	bool AreTheRigidBodiesCollidingVertically(RigidBody* rb1, RigidBody* rb2, bool just_check_no_response = false);
public:
	void OnlyCollideWithPlatforms(bool only_colliding_with_platforms_now);
	bool IsFacingRight();
	bool IsInTheAir();
	bool WasFacingRightWhenHit();
	void SetFacingRightWhenHit(bool facing_right_when_hit);
	void SetInTheAir(bool in_the_air_now);
	sf::Vector2f GetVelocity();
	sf::Vector2f GetCurrentPosition();
	sf::Vector2f GetCurrentDimensions();
	std::vector<int> GetEntitiesExcludedFromCollision();
	bool WillOnlyCollidingWithPlatforms();
	int GetEntityType();
	void EnableCollision();
	void DisableCollision();
	void EnableGravity();
	void DisableGravity();
	void SetVelocity(sf::Vector2f new_velocity);
	void SetVelocity(float x, float y);
	void SetCurrentPosition(sf::Vector2f new_position);
	void SetCurrentPosition(float x, float y);
	void SetEntityType(int new_entity_type);
	int GetID();
	int GetGridTopLeftX();
	int GetGridTopLeftY();
	int GetGridBotRightX();
	int GetGridBotRightY();
	void SetGridTopLeftX(int new_value);
	void SetGridTopLeftY(int new_value);
	void SetGridBotRightX(int new_value);
	void SetGridBotRightY(int new_value);
	void LockFacingDirection();
	void UnlockFacingDirection();
	bool IsFacingDirectionLocked();
	RigidBody(sf::Vector2f position, sf::Vector2f dimensions, bool subject_to_gravity = false, bool subject_to_collision = true);
	virtual void Update(sf::Int64 curr_time, sf::Int64 delta_time);
	void ChangeFutureValuesAndVelocityBasedOnCollisions();
	std::vector<RigidBody*> GetCollidersRigidBodyIsCollidingWith();
	static float GetDistanceBetweenTwoPoints(sf::Vector2f point_a, sf::Vector2f point_b);
	void ExcludeFromCollision(int ent_typ);
	void ClearEntitiesExcludedFromCollision();
};

#endif