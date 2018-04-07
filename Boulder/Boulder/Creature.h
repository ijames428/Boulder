#pragma once

#ifndef CREATURE_H
#define CREATURE_H

using namespace std;
//#include "..\GameLibrary\Singleton.h"
#include "PlatformerLibrary\RigidBody.h"
#include "PlatformerLibrary\Box2DRigidBody.h"
//#include "World.h"
//#include <SFML/Graphics.hpp>
//#include "Projectile.h"

class Creature : public Box2DRigidBody {
	protected:
		sf::Int64 hit_stun_start_time;
		sf::Int64 hit_stun_duration;
		float speed;
		float jump_power;
		sf::Int64 post_hit_invincibility_start_time;
		sf::Int64 post_hit_invincibility_duration;
		sf::Int64 dodge_start_time;
		sf::Int64 dodge_duration;
		sf::Int64 dodge_invincibility_start_time;
		sf::Int64 dodge_invincibility_duration;
		float dodge_height;
		float usual_height;
		float dodge_velocity_x;
	public:
		sf::Int16 hit_points;
		sf::Int16 max_hit_points;
		Creature(sf::RenderWindow *window, sf::Vector2f position = sf::Vector2f(0.0f, 0.0f), sf::Vector2f dimensions = sf::Vector2f(0.0f, 0.0f), bool subject_to_gravity = true);
		sf::RectangleShape rectangle_shape;
		void Draw(sf::Vector2f camera_position);
		void TakeHit(sf::Int64 damage, sf::Int64 hit_stun_duration, sf::Vector2f knock_back, bool activate_invincibility = true, bool lock_facing_direction = false);
		void OnDeath();
		bool IsInPostHitInvincibility();
		bool IsInRollInvincibility();
		bool IsInvincible();
		bool IsDodging();
		virtual void Update(sf::Int64 curr_frame, sf::Int64 delta_time);
};

#endif