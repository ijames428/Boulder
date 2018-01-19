#pragma once

using namespace std;
#include "stdafx.h"
#include <iostream>
#include "SmashCharacter.h"
#include "Constants.h"
#include "GameLibrary\InputHandler.h"
#include "SmashWorld.h"

SmashCharacter::SmashCharacter(int player_idx, Json::Value playerBestiaryData, sf::RenderWindow *window, sf::Vector2f position, sf::Vector2f dimensions, bool subject_to_gravity) :
	BoulderCreature::BoulderCreature(player_idx, window, position, dimensions, subject_to_gravity) {
	SetEntityType(Constants::ENTITY_TYPE_PLAYER_CHARACTER);
	name = "Player";
	player_index = player_idx;
	is_interactable = false;

	hit_points = max_hit_points = playerBestiaryData["DictOfUnits"]["Player"]["HitPoints"].asInt();
	can_take_input = true;

	speed = 5.0f;
	running_speed_multiplier = 2.0f;
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

	topCircleFixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->PLAYER_CHARACTER;
	botCircleFixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->PLAYER_CHARACTER;
	centerBoxFixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->PLAYER_CHARACTER;

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

	numberOfAttacks = (int)Attack::MOVES_COUNT;// (int)playerBestiaryData["AttackingAnimations"].size();
	attacks.push_back(new Attack(body, player_index, Attack::JAB, playerBestiaryData["DictOfUnits"]["Player"]["AttackingAnimations"][Attack::JAB]));
	attacks.push_back(new Attack(body, player_index, Attack::UP_SMASH, playerBestiaryData["DictOfUnits"]["Player"]["AttackingAnimations"][Attack::UP_SMASH]));
	attacks.push_back(new Attack(body, player_index, Attack::DOWN_SMASH, playerBestiaryData["DictOfUnits"]["Player"]["AttackingAnimations"][Attack::DOWN_SMASH]));
	attacks.push_back(new Attack(body, player_index, Attack::FORWARD_SMASH, playerBestiaryData["DictOfUnits"]["Player"]["AttackingAnimations"][Attack::FORWARD_SMASH]));
	attacks.push_back(new Attack(body, player_index, Attack::UP_AIR, playerBestiaryData["DictOfUnits"]["Player"]["AttackingAnimations"][Attack::UP_AIR]));
	attacks.push_back(new Attack(body, player_index, Attack::DOWN_AIR, playerBestiaryData["DictOfUnits"]["Player"]["AttackingAnimations"][Attack::DOWN_AIR]));
	attacks.push_back(new Attack(body, player_index, Attack::FORWARD_AIR, playerBestiaryData["DictOfUnits"]["Player"]["AttackingAnimations"][Attack::FORWARD_AIR]));
	attacks.push_back(new Attack(body, player_index, Attack::BACK_AIR, playerBestiaryData["DictOfUnits"]["Player"]["AttackingAnimations"][Attack::BACK_AIR]));
	attacks.push_back(new Attack(body, player_index, Attack::NEUTRAL_AIR, playerBestiaryData["DictOfUnits"]["Player"]["AttackingAnimations"][Attack::NEUTRAL_AIR]));

	hit_stun_timer = new StatusTimer(1);
	jump_input_buffer = new StatusTimer(6);

	weapon = new Weapon(window, position, sf::Vector2f(0.25f, 0.25f), true, player_index, body);// , this);
	
	starting_health_bar_width = 240.0f;
	healthBarRect = new sf::RectangleShape(sf::Vector2f(starting_health_bar_width, 15.0f));
	healthBarRect->setPosition(10.0f, 10.0f);
	healthBarRect->setFillColor(sf::Color::Green);

	healthBarBackgroundRect = new sf::RectangleShape(sf::Vector2f(250.0f, 25.0f));
	healthBarBackgroundRect->setPosition(5.0f, 5.0f);
	healthBarBackgroundRect->setFillColor(sf::Color::Black);

	sprite_scale = 1.0f;

	LoadAllAnimations("Player", playerBestiaryData);

	int numberOfLandingAnimationFrames = landing_animations.size() > 0 ? landing_animations[0]->GetNumberOfFrames() : 1;
	landing_animation_timer = new StatusTimer(numberOfLandingAnimationFrames);

	is_hittable = true;
}

void SmashCharacter::ReceiveHeal(int heal) {
	BoulderCreature::ReceiveHeal(heal);
	UpdateHealthBar();
}

void SmashCharacter::UpdateHealthBar() {
	float percent_health = ((float)hit_points / (float)max_hit_points);

	healthBarRect->setSize(sf::Vector2f(starting_health_bar_width * percent_health, 15.0f));
	healthBarRect->setPosition(10.0f, 10.0f);
	healthBarRect->setFillColor(sf::Color((int)(0.0f + (255.0f - (float)hit_points / (float)max_hit_points) * 255.0f), (int)((float)hit_points / (float)max_hit_points * 255.0f), 0, 255));
}

void SmashCharacter::TakeDamage(int damage, sf::Vector2f knock_back, int hit_stun_frames) {
	BoulderCreature::TakeDamage(damage, knock_back, hit_stun_frames);
	UpdateHealthBar();
}

void SmashCharacter::ApplyObjectDataToSaveData(Json::Value& save_data) {
	BoulderCreature::ApplyObjectDataToSaveData(save_data["Player"]);
}

void SmashCharacter::ApplySaveDataToObjectData(Json::Value& save_data) {
	BoulderCreature::ApplySaveDataToObjectData(save_data["Player"]);
}

void SmashCharacter::Update(sf::Int64 curr_frame, sf::Int64 delta_time) {
	Creature::Update(curr_frame, delta_time);

	can_take_input = !hit_stun_timer->IsActive();
	if (can_take_input) {
		can_take_input = !IsAnAttackActive();
	}

	//if (body->GetLinearVelocity().x > 0.1f && !IsFacingRight()) {
	//	SetFacingRight(true);
	//} else if (body->GetLinearVelocity().x < -0.1f && IsFacingRight()) {
	//	SetFacingRight(false);
	//}

	for (int i = 0; i < numberOfAttacks; i++) {
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

	if (hit_points <= 0) {
		if (dying_animation_timer != nullptr && dying_animation_timer->IsActive()) {
			State = STATE_DYING;
		} else {
			State = STATE_DEAD;
		}
	} else if (hit_stun_timer->IsActive()) {
		State = STATE_HIT_STUN;
	} else if (IsAnAttackActive()) {
		State = STATE_ATTACKING;
	} else if (IsInTheAir()) {
		if (body->GetLinearVelocity().y < -1.0f) {
			State = STATE_JUMPING;
		} else if (body->GetLinearVelocity().y > 1.0f) {
			State = STATE_FALLING;
		} else {
			State = STATE_JUMP_APEX;
		}
	} else if (landing_animation_timer->IsActive()) {
		State = STATE_LANDING;
	} else if (body->GetLinearVelocity().x != 0) {
		if (running) {
			State = STATE_RUNNING;
		} else {
			State = STATE_WALKING;
		}
	} else {
		State = STATE_IDLE;
	}

	if (State == STATE_WALKING) {
		if (walking_animations[0]->GetCurrentFrame() == RightFootStepSoundFrameWalk) {
			RightFootStepSound.setVolume(25);
			RightFootStepSound.play();
		} else if (walking_animations[0]->GetCurrentFrame() == LeftFootStepSoundFrameWalk) {
			LeftFootStepSound.setVolume(25);
			LeftFootStepSound.play();
		}
	} else if (State == STATE_RUNNING) {
		if (running_animations[0]->GetCurrentFrame() == RightFootStepSoundFrameRun) {
			RightFootStepSound.setVolume(60);
			RightFootStepSound.play();
		} else if (running_animations[0]->GetCurrentFrame() == LeftFootStepSoundFrameRun) {
			LeftFootStepSound.setVolume(60);
			LeftFootStepSound.play();
		}
	} else if (State == STATE_ATTACKING) {
		int currentAttackIndex = GetActiveAttackIndex();
		if (currentAttackIndex < (int)AttackAnimationSounds.size()) {
			if (attacking_animations[currentAttackIndex]->GetCurrentFrame() == AttackAnimationSoundFrames[currentAttackIndex]) {
				AttackAnimationSounds[currentAttackIndex]->play();
			}
		}
	}
}

void SmashCharacter::Draw(sf::Vector2f camera_position) {
	render_window->draw(*healthBarBackgroundRect);

	BoulderCreature::Draw(camera_position);
}

void SmashCharacter::HandleButtonXPress() {
	if (interactable != nullptr) {
		interactable->StartTalking();
		Singleton<SmashWorld>::Get()->StartDialogue(interactable->GetType());
	} else {
		if (IsInTheAir()) {
			UseAttack(Attack::NEUTRAL_AIR);
		} else {
			UseAttack(Attack::JAB);
		}
	}
}

void SmashCharacter::DashPunch() {
}

void SmashCharacter::ThrowWeapon() {
	if (IsFacingRight()) {
		weapon->Throw(b2Vec2(5.0f + body->GetLinearVelocity().x, -5.0f + body->GetLinearVelocity().y), body->GetPosition());
	} else {
		weapon->Throw(b2Vec2(-5.0f + body->GetLinearVelocity().x, -5.0f + body->GetLinearVelocity().y), body->GetPosition());
	}
}

void SmashCharacter::TeleportToWeapon() {
	b2Vec2 original_position = body->GetPosition();
	b2Vec2 new_position = weapon->GetBody()->GetPosition();

	b2Vec2 travel_vector = b2Vec2(new_position.x - original_position.x, new_position.y - original_position.y);
	travel_vector.Normalize();
	float32 speed = 10.0f;

	body->SetTransform(weapon->GetBody()->GetPosition(), body->GetAngle());
	body->SetLinearVelocity(b2Vec2(travel_vector.x * speed, travel_vector.y * speed));

	weapon->TeleportedTo();
}

void SmashCharacter::HandleButtonSelectPress() {
	Singleton<SmashWorld>::Get()->HandleButtonSelectPress();
}

void SmashCharacter::HandleButtonSelectRelease() {
	Singleton<SmashWorld>::Get()->HandleButtonSelectRelease();
}

void SmashCharacter::HandleButtonStartPress() {
	Singleton<SmashWorld>::Get()->HandleButtonStartPress();
}

void SmashCharacter::HandleButtonStartRelease() {
	Singleton<SmashWorld>::Get()->HandleButtonStartRelease();
}