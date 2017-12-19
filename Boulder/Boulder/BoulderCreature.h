#pragma once

#ifndef BOULDER_CHARACTER_H
#define BOULDER_CHARACTER_H

using namespace std;
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
//#include "..\PlatformerLibrary\RigidBody.h"
#include "GameLibrary\SpriteAnimation.h"
#include "GameLibrary\Settings.h"
//#include "..\GameLibrary\AssetManager.h"
//#include "World.h"
#include "Creature.h"
#include "Projectile.h"
#include "PlatformerLibrary\Box2D\Box2D.h"
#include "Attack.h"
#include "StatusTimer.h"

class BoulderCreature : public Creature {
private:
protected:
	BoulderCreature(int player_idx, sf::RenderWindow *window, sf::Vector2f position = sf::Vector2f(0.0f, 0.0f), sf::Vector2f dimensions = sf::Vector2f(0.0f, 0.0f), bool subject_to_gravity = true);
	std::vector<Attack*> attacks = std::vector<Attack*>();
	int player_index;
	bool can_take_input;
	int max_hit_points;
	sf::Color player_color;
	b2BodyDef bodyDef;
	b2Body* body;
	b2CircleShape topCircleShape;
	b2CircleShape botCircleShape;
	b2PolygonShape centerBoxShape;
	b2FixtureDef topCircleFixtureDef;
	b2FixtureDef botCircleFixtureDef;
	b2FixtureDef centerBoxFixtureDef;
	b2Fixture* topCircleFixture;
	b2Fixture* botCircleFixture;
	b2Fixture* centerBoxFixture;
	StatusTimer* hit_stun_timer;
	StatusTimer* jump_input_buffer;
	b2PolygonShape groundCheckShape;
	b2FixtureDef groundCheckFixtureDef;
	bool has_double_jump;
	void Move(float horizontal, float vertical);
	void Jump();
	void UseAttack(int move_type);
	sf::RectangleShape* healthBarRect;
	float healthBarStartingScaleY;
	string name = "";
public:
	BoulderCreature(sf::RenderWindow *window, sf::Vector2f position = sf::Vector2f(0.0f, 0.0f), sf::Vector2f dimensions = sf::Vector2f(0.0f, 0.0f), bool subject_to_gravity = true);
	void Draw(sf::Vector2f camera_position);
	virtual void Update(sf::Int64 curr_frame, sf::Int64 delta_time);
	void TakeDamage(int damage, sf::Vector2f knock_back, int hit_stun_frames);
	Attack* GetActiveAttack();
	bool IsAnAttackActive();
	void Land();

	string GetName() {
		return name;
	};
	void SetName(string new_name) {
		name = new_name;
	};
	b2Body* GetBody() {
		return body;
	}
	virtual int GetHitPoints() {
		return hit_points;
	};
};

#endif