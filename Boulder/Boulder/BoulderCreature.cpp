#pragma once

using namespace std;
#include "stdafx.h"
#include <iostream>
#include "BoulderCreature.h"
#include "Constants.h"
#include "SmashWorld.h"

BoulderCreature::BoulderCreature(int player_idx, sf::RenderWindow *window, sf::Vector2f position, sf::Vector2f dimensions, bool subject_to_gravity) :
	Creature::Creature(window, position, dimensions, subject_to_gravity) {
}

BoulderCreature::BoulderCreature(sf::RenderWindow *window, sf::Vector2f position, sf::Vector2f dimensions, bool subject_to_gravity) :
	Creature::Creature(window, position, dimensions, subject_to_gravity) {
	SetEntityType(Constants::ENTITY_TYPE_CREATURE);

	player_index = 1;

	hit_points = max_hit_points = 100;
	can_take_input = false;

	speed = 5.0f;
	jump_power = 10.0f;

	sf::RectangleShape shape(dimensions);
	shape.setFillColor(sf::Color::Yellow);
	shape.setPosition(position);
	rectangle_shape = shape;
	player_color = sf::Color::Cyan;

	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(position.x, position.y);
	body = Singleton<SmashWorld>::Get()->GetB2World()->CreateBody(&bodyDef);

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

	topCircleFixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->PLAYER_TWO;
	botCircleFixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->PLAYER_TWO;
	centerBoxFixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->PLAYER_TWO;

	topCircleFixture = body->CreateFixture(&topCircleFixtureDef);
	botCircleFixture = body->CreateFixture(&botCircleFixtureDef);
	centerBoxFixture = body->CreateFixture(&centerBoxFixtureDef);
	body->SetFixedRotation(true);
	body->SetUserData(this);

	groundCheckShape.SetAsBox(dimensions.x / 2.0f, 0.05f, b2Vec2(0.0f, 0.6f), 0.0f);
	groundCheckFixtureDef.shape = &groundCheckShape;
	groundCheckFixtureDef.isSensor = true;
	groundCheckFixtureDef.m_color = new b2Color(1.0f, 0.0f, 1.0f, 1.0f);

	groundCheckFixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->GROUND_CHECK;
	groundCheckFixtureDef.filter.maskBits = Singleton<SmashWorld>::Get()->PLATFORM;

	body->CreateFixture(&groundCheckFixtureDef);

	//attacks.push_back(new Attack(body, player_index, Attack::JAB));
	//attacks.push_back(new Attack(body, player_index, Attack::UP_SMASH));
	//attacks.push_back(new Attack(body, player_index, Attack::DOWN_SMASH));
	//attacks.push_back(new Attack(body, player_index, Attack::FORWARD_SMASH));
	//attacks.push_back(new Attack(body, player_index, Attack::UP_AIR));
	//attacks.push_back(new Attack(body, player_index, Attack::DOWN_AIR));
	//attacks.push_back(new Attack(body, player_index, Attack::FORWARD_AIR));
	//attacks.push_back(new Attack(body, player_index, Attack::BACK_AIR));
	//attacks.push_back(new Attack(body, player_index, Attack::NEUTRAL_AIR));
	//attacks.push_back(new Attack(body, player_index, Attack::DASH_ATTACK));
	//attacks.push_back(new Attack(body, player_index, Attack::DASH_PUNCH));
	//attacks.push_back(new Attack(body, player_index, Attack::THROW_WEAPON));
	//attacks.push_back(new Attack(body, player_index, Attack::TELEPORT_TO_WEAPON));
	//attacks.push_back(new Attack(body, player_index, Attack::URIENS_SUPER));

	hit_stun_timer = new StatusTimer(1);
	jump_input_buffer = new StatusTimer(6);

	healthBarRect = new sf::RectangleShape(sf::Vector2f(5, 800));
	healthBarRect->setPosition(10.0f, 10.0f);
	healthBarRect->setFillColor(sf::Color::Green);
}

void BoulderCreature::Update(sf::Int64 curr_frame, sf::Int64 delta_time) {
	Creature::Update(curr_frame, delta_time);

	can_take_input = !hit_stun_timer->IsActive();
	if (can_take_input) {
		can_take_input = !IsAnAttackActive();
	}

	if (body->GetLinearVelocity().x > 0.1f && !IsFacingRight()) {
		SetFacingRight(true);
	}
	else if (body->GetLinearVelocity().x < -0.1f && IsFacingRight()) {
		SetFacingRight(false);
	}

	for (int i = 0; i < (int)attacks.size(); i++) {
		attacks[i]->Update(curr_frame, IsFacingRight());
	}

	if (hit_stun_timer->IsActive()) {
		topCircleFixture->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
		botCircleFixture->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
		centerBoxFixture->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
	}
	else {
		topCircleFixture->SetColor(0.0f, 1.0f, 0.0f, 1.0f);
		botCircleFixture->SetColor(0.0f, 1.0f, 0.0f, 1.0f);
		centerBoxFixture->SetColor(0.0f, 1.0f, 0.0f, 1.0f);
	}
}

void BoulderCreature::Draw(sf::Vector2f camera_position) {
	render_window->draw(*healthBarRect);
}

void BoulderCreature::Move(float horizontal, float vertical) {
	if (can_take_input && hit_points > 0) {
		body->SetLinearVelocity(b2Vec2((horizontal / 100.0f) * speed, body->GetLinearVelocity().y));

		if (horizontal > 0) {
			SetFacingRight(true);
		}
		else if (horizontal < 0) {
			SetFacingRight(false);
		}
	}
}

void BoulderCreature::Jump() {
	if (can_take_input && hit_points > 0) {
		bool jumping = false;

		if (!IsInTheAir()) {
			jumping = true;
		}
		else if (has_double_jump) {
			jumping = true;
			has_double_jump = false;
		}

		if (jumping) {
			body->SetLinearVelocity(b2Vec2(body->GetLinearVelocity().x, -jump_power));
			SetInTheAir(true);
		}
	}
}

void BoulderCreature::Land() {
	SetInTheAir(false);
	has_double_jump = true;
	if (IsAnAttackActive()) {
		GetActiveAttack()->StopAttack();
	}
	if (jump_input_buffer->IsActive()) {
		Jump();
	}
}

void BoulderCreature::UseAttack(int move_type) {
	if (can_take_input && hit_points > 0) {
		attacks[move_type]->InitiateAttack();
	}
}

void BoulderCreature::TakeDamage(int damage, sf::Vector2f knock_back, int hit_stun_frames) {
	cout << "got hit for " << damage << " damage!\n";
	hit_stun_timer = new StatusTimer(hit_stun_frames);
	hit_stun_timer->Start();
	body->SetLinearVelocity(b2Vec2(knock_back.x, knock_back.y));
}

Attack* BoulderCreature::GetActiveAttack() {
	Attack* active_attack = attacks[Attack::JAB];

	for (int i = 0; i < (int)attacks.size(); i++) {
		if (attacks[i]->IsAttacking()) {
			active_attack = attacks[i];
		}
	}

	return active_attack;
}

bool BoulderCreature::IsAnAttackActive() {
	for (int i = 0; i < (int)attacks.size(); i++) {
		if (attacks[i]->IsAttacking()) {
			return true;
		}
	}

	return false;
}