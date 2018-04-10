#pragma once

using namespace std;
#include "stdafx.h"
#include <iostream>
#include "Creature.h"
#include "Constants.h"

Creature::Creature(sf::RenderWindow *window, sf::Vector2f position, sf::Vector2f dimensions, bool subject_to_gravity) : Box2DRigidBody(window, subject_to_gravity) {
	SetEntityType(Constants::ENTITY_TYPE_CREATURE);
	hit_points = 1;
	SetFacingRightWhenHit(false);
	UnlockFacingDirection();
	post_hit_invincibility_start_time = 0;
	post_hit_invincibility_duration = 2000;

	dodge_start_time = 0;
	dodge_duration = 300;
	dodge_invincibility_start_time = 0;
	dodge_invincibility_duration = 150;
	dodge_height = dimensions.y / 2.0f;
	usual_height = dimensions.y;
	dodge_velocity_x = 6.0f;

	speed = 2.0f;
	jump_power = 1.0f;

	current_frame = 0;
	hit_stun_start_time = 0;
	hit_stun_duration = 0;

	render_window = window;

	sf::RectangleShape shape(dimensions);
	shape.setFillColor(sf::Color::Red);
	shape.setPosition(position);

	rectangle_shape = shape;
}

void Creature::Update(sf::Int64 curr_frame, sf::Int64 delta_time) {
	Box2DRigidBody::Update(curr_frame, delta_time);
}

void Creature::Draw(sf::Vector2f camera_position) {
	rectangle_shape.setPosition(sf::Vector2f(GetCurrentPosition().x - camera_position.x, GetCurrentPosition().y - camera_position.y));
	render_window->draw(rectangle_shape);
}

bool Creature::IsDodging() {
	return dodge_start_time + dodge_duration > current_frame;
}

bool Creature::IsInvincible() {
	return IsInRollInvincibility() || IsInPostHitInvincibility();
}

bool Creature::IsInRollInvincibility() {
	return dodge_invincibility_start_time + dodge_invincibility_duration > current_frame;
}

bool Creature::IsInPostHitInvincibility() {
	return post_hit_invincibility_start_time + post_hit_invincibility_duration > current_frame;
}

void Creature::TakeHit(sf::Int64 damage, sf::Int64 hit_stun_dur, sf::Vector2f knock_back, bool activate_invincibility, bool lock_facing_direction) {
#ifdef _DEBUG
	if (GetEntityType() == Constants::ENTITY_TYPE_PLAYER_CHARACTER) {
		//damage = 0;
	}
#endif

	if (hit_points <= 0) {
		return;
	}

	if (GetEntityType() == Constants::ENTITY_TYPE_PLAYER_CHARACTER) {
		if (IsInPostHitInvincibility()) {
			return;
		}

		if (activate_invincibility) {
			post_hit_invincibility_start_time = current_frame;
		}
	}

	if (lock_facing_direction) {
		SetFacingRightWhenHit(IsFacingRight());
		LockFacingDirection();
	}

	//SetVelocity(knock_back.x, -knock_back.y);
	
	sf::Int16 adjusted_damage = (sf::Int16)damage;

	if (hit_points - adjusted_damage <= 0) {
		OnDeath();
	} else {
		hit_points = hit_points - adjusted_damage;
	}

	sf::Time sleep_time = sf::milliseconds(20);
	sf::sleep(sf::Time(sleep_time));
	if (hit_stun_start_time + hit_stun_duration < hit_stun_dur + current_frame) {
		hit_stun_duration = hit_stun_dur;
		hit_stun_start_time = current_frame;
	}
}

void Creature::OnDeath() {
	hit_points = 0;

	if (GetEntityType() != Constants::ENTITY_TYPE_PLAYER_CHARACTER) {
		//OnlyCollideWithPlatforms(true);
		//ExcludeFromCollision(Constants::ENTITY_TYPE_PLAYER_CHARACTER);
		//ExcludeFromCollision(Constants::ENTITY_TYPE_RIGID_BODY);
		//ExcludeFromCollision(Constants::ENTITY_TYPE_DRONE);
		//ExcludeFromCollision(Constants::ENTITY_TYPE_GRUNT);
		//ExcludeFromCollision(Constants::ENTITY_TYPE_GUNNER);
		//ExcludeFromCollision(Constants::ENTITY_TYPE_CHARGER);
		//ExcludeFromCollision(Constants::ENTITY_TYPE_PROJECTILE);
		//ExcludeFromCollision(Constants::ENTITY_TYPE_HIT_BOX);
	} else {
		//Singleton<World>::Get()->PlayerDied();
	}
}