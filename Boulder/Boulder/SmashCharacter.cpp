#pragma once

using namespace std;
#include "stdafx.h"
#include <iostream>
#include "SmashCharacter.h"
#include "Constants.h"
#include "GameLibrary\InputHandler.h"
#include "SmashWorld.h"

SmashCharacter::SmashCharacter(int player_idx, sf::RenderWindow *window, sf::Vector2f position, sf::Vector2f dimensions, bool subject_to_gravity) :
	Creature::Creature(window, position, dimensions, subject_to_gravity) {
	SetEntityType(Constants::ENTITY_TYPE_PLAYER_CHARACTER);
	player_index = player_idx;

	hit_points = max_hit_points = 10;
	can_take_input = true;

	speed = 5.0f;
	jump_power = 10.0f;

	sf::RectangleShape shape(dimensions);
	shape.setFillColor(sf::Color::Yellow);
	shape.setPosition(position);
	rectangle_shape = shape;
	player_color = sf::Color::Cyan;

	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(position.x, position.y);
	playerBody = Singleton<SmashWorld>::Get()->GetB2World()->CreateBody(&bodyDef);

	//b2CircleShape topCircleShape;
	//b2CircleShape botCircleShape;
	//b2PolygonShape centerBoxShape;
	//b2FixtureDef topCircleFixtureDef;
	//b2FixtureDef botCircleFixtureDef;
	//b2FixtureDef centerBoxFixtureDef;
	//b2Fixture* topCircleFixture;
	//b2Fixture* botCircleFixture;
	//b2Fixture* centerBoxFixture;

	centerBoxShape.SetAsBox(dimensions.x / 4.0f - 0.01f, 0.5f);
	topCircleShape.m_p.Set(0, -0.5f); //position, relative to body position
	botCircleShape.m_p.Set(0, 0.5f); //position, relative to body position
	topCircleShape.m_radius = dimensions.x / 4.0f;
	botCircleShape.m_radius = dimensions.x / 4.0f;
	centerBoxFixtureDef.shape = &centerBoxShape;
	centerBoxFixtureDef.density = 1.0f;
	centerBoxFixtureDef.friction = 0.3f;
	centerBoxFixtureDef.m_color = new b2Color(0.0f, 1.0f, 0.0f, 1.0f);
	topCircleFixtureDef.shape = &topCircleShape;
	topCircleFixtureDef.density = 1.0f;
	topCircleFixtureDef.friction = 0.01f;
	topCircleFixtureDef.m_color = new b2Color(0.0f, 1.0f, 0.0f, 1.0f);
	botCircleFixtureDef.shape = &botCircleShape;
	botCircleFixtureDef.density = 1.0f;
	botCircleFixtureDef.friction = 0.3f;
	botCircleFixtureDef.m_color = new b2Color(0.0f, 1.0f, 0.0f, 1.0f);

	if (player_index == 0) {
		topCircleFixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->PLAYER_ONE;
		botCircleFixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->PLAYER_ONE;
		centerBoxFixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->PLAYER_ONE;
	} else {
		topCircleFixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->PLAYER_TWO;
		botCircleFixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->PLAYER_TWO;
		centerBoxFixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->PLAYER_TWO;
	}

	topCircleFixture = playerBody->CreateFixture(&topCircleFixtureDef);
	botCircleFixture = playerBody->CreateFixture(&botCircleFixtureDef);
	centerBoxFixture = playerBody->CreateFixture(&centerBoxFixtureDef);
	playerBody->SetFixedRotation(true);
	playerBody->SetUserData(this);

	groundCheckShape.SetAsBox(dimensions.x / 2.0f, 0.05f, b2Vec2(0.0f, 0.6f), 0.0f);
	groundCheckFixtureDef.shape = &groundCheckShape;
	groundCheckFixtureDef.isSensor = true;
	groundCheckFixtureDef.m_color = new b2Color(1.0f, 0.0f, 1.0f, 1.0f);

	//if (player_index == 0) {
	//	groundCheckFixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->GROUND_CHECK_ONE;
	//} else {
	//	groundCheckFixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->GROUND_CHECK_TWO;
	//}
	groundCheckFixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->GROUND_CHECK;
	groundCheckFixtureDef.filter.maskBits = Singleton<SmashWorld>::Get()->PLATFORM;

	playerBody->CreateFixture(&groundCheckFixtureDef);

	attacks[Attack::JAB] = new Attack(playerBody, player_index, Attack::JAB);
	attacks[Attack::UP_SMASH] = new Attack(playerBody, player_index, Attack::UP_SMASH);
	attacks[Attack::DOWN_SMASH] = new Attack(playerBody, player_index, Attack::DOWN_SMASH);
	attacks[Attack::FORWARD_SMASH] = new Attack(playerBody, player_index, Attack::FORWARD_SMASH);
	attacks[Attack::UP_AIR] = new Attack(playerBody, player_index, Attack::UP_AIR);
	attacks[Attack::DOWN_AIR] = new Attack(playerBody, player_index, Attack::DOWN_AIR);
	attacks[Attack::FORWARD_AIR] = new Attack(playerBody, player_index, Attack::FORWARD_AIR);
	attacks[Attack::BACK_AIR] = new Attack(playerBody, player_index, Attack::BACK_AIR);
	attacks[Attack::NEUTRAL_AIR] = new Attack(playerBody, player_index, Attack::NEUTRAL_AIR);
	attacks[Attack::DASH_ATTACK] = new Attack(playerBody, player_index, Attack::DASH_ATTACK);
	attacks[Attack::DASH_PUNCH] = new Attack(playerBody, player_index, Attack::DASH_PUNCH);
	attacks[Attack::THROW_WEAPON] = new Attack(playerBody, player_index, Attack::THROW_WEAPON);
	attacks[Attack::TELEPORT_TO_WEAPON] = new Attack(playerBody, player_index, Attack::TELEPORT_TO_WEAPON);
	attacks[Attack::URIENS_SUPER] = new Attack(playerBody, player_index, Attack::URIENS_SUPER);

	hit_stun_timer = new StatusTimer(1);

	weapon = new Weapon(window, position, sf::Vector2f(0.25f, 0.25f), true, player_index, playerBody);
}

void SmashCharacter::Update(sf::Int64 curr_frame, sf::Int64 delta_time) {
	Creature::Update(curr_frame, delta_time);

	can_take_input = !hit_stun_timer->IsActive();
	if (can_take_input) {
		can_take_input = !IsAnAttackActive();
	}

	if (playerBody->GetLinearVelocity().x > 0.1f && !IsFacingRight()) {
		SetFacingRight(true);
	} else if (playerBody->GetLinearVelocity().x < -0.1f && IsFacingRight()) {
		SetFacingRight(false);
	}

	for (int i = 0; i < Attack::Moves::MOVES_COUNT; i++) {
		attacks[i]->Update(curr_frame, IsFacingRight());
	}

	weapon->Update(curr_frame, delta_time);

	if (hit_stun_timer->IsActive()) {
		topCircleFixture->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
		botCircleFixture->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
		centerBoxFixture->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
	} else {
		topCircleFixture->SetColor(0.0f, 1.0f, 0.0f, 1.0f);
		botCircleFixture->SetColor(0.0f, 1.0f, 0.0f, 1.0f);
		centerBoxFixture->SetColor(0.0f, 1.0f, 0.0f, 1.0f);
	}
}

void SmashCharacter::Draw(sf::Vector2f camera_position) {
}

void SmashCharacter::Move(float horizontal, float vertical) {
	if (can_take_input && hit_points > 0) {
		playerBody->SetLinearVelocity(b2Vec2((horizontal / 100.0f) * speed, playerBody->GetLinearVelocity().y));

		if (horizontal > 0) {
			SetFacingRight(true);
		} else if (horizontal < 0) {
			SetFacingRight(false);
		}
	}
}

void SmashCharacter::Jump() {
	if (can_take_input && hit_points > 0) {
		bool jumping = false;

		if (!IsInTheAir()) {
			jumping = true;
		} else if (has_double_jump) {
			jumping = true;
			has_double_jump = false;
		}

		if (jumping) {
			playerBody->SetLinearVelocity(b2Vec2(playerBody->GetLinearVelocity().x, -jump_power));
			SetInTheAir(true);
		}
	}
}

void SmashCharacter::Land() {
	SetInTheAir(false);
	has_double_jump = true;
}

void SmashCharacter::DashPunch() {
}

void SmashCharacter::ThrowWeapon() {
	if (IsFacingRight()) {
		weapon->Throw(b2Vec2(5.0f + playerBody->GetLinearVelocity().x, -5.0f + playerBody->GetLinearVelocity().y), playerBody->GetPosition());
	} else {
		weapon->Throw(b2Vec2(-5.0f + playerBody->GetLinearVelocity().x, -5.0f + playerBody->GetLinearVelocity().y), playerBody->GetPosition());
	}
}

void SmashCharacter::TeleportToWeapon() {
	b2Vec2 original_position = playerBody->GetPosition();
	b2Vec2 new_position = weapon->GetBody()->GetPosition();

	b2Vec2 travel_vector = b2Vec2(new_position.x - original_position.x, new_position.y - original_position.y);
	travel_vector.Normalize();
	float32 speed = 10.0f;

	playerBody->SetTransform(weapon->GetBody()->GetPosition(), playerBody->GetAngle());
	playerBody->SetLinearVelocity(b2Vec2(travel_vector.x * speed, travel_vector.y * speed));

	weapon->TeleportedTo();

}

void SmashCharacter::UseAttack(int move_type) {
	if (can_take_input && hit_points > 0) {
		attacks[move_type]->InitiateAttack();
	}
}

void SmashCharacter::TakeDamage(int damage, sf::Vector2f knock_back, int hit_stun_frames) {
	cout << "got hit for " << damage << " damage!\n";
	hit_stun_timer = new StatusTimer(hit_stun_frames);
	hit_stun_timer->Start();
	playerBody->SetLinearVelocity(b2Vec2(knock_back.x, knock_back.y));
}

Attack* SmashCharacter::GetActiveAttack() {
	Attack* active_attack = attacks[Attack::JAB];

	for (int i = 0; i < Attack::Moves::MOVES_COUNT; i++) {
		if (attacks[i]->IsAttacking()) {
			active_attack = attacks[i];
		}
	}

	return active_attack;
}

bool SmashCharacter::IsAnAttackActive() {
	for (int i = 0; i < Attack::Moves::MOVES_COUNT; i++) {
		if (attacks[i]->IsAttacking()) {
			return true;
		}
	}

	return false;
}