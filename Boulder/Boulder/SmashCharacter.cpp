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
	can_take_input = true;

	hit_points = max_hit_points = playerBestiaryData["DictOfUnits"]["Player"]["HitPoints"].asInt();

	speed = 5.0f;
	running_speed_multiplier = 2.0f;
	jump_power = 10.0f;
	attacksAreInterruptible = true;
	teleportedSinceLastLanding = false;
	rightStickWasCentered = true;

	leftStickInputs = sf::Vector2f(0.0f, 0.0f);

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
	centerBoxFixtureDef.friction = 0.0f;
	centerBoxFixtureDef.m_color = new b2Color(0.0f, 1.0f, 0.0f, 1.0f);
	topCircleFixtureDef.shape = &topCircleShape;
	topCircleFixtureDef.density = 1.0f;
	topCircleFixtureDef.friction = 0.0f;
	topCircleFixtureDef.m_color = new b2Color(0.0f, 1.0f, 0.0f, 1.0f);
	botCircleFixtureDef.shape = &botCircleShape;
	botCircleFixtureDef.density = 1.0f;
	botCircleFixtureDef.friction = 1.0f;
	botCircleFixtureDef.m_color = new b2Color(0.0f, 1.0f, 0.0f, 1.0f);

	topCircleFixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->PLAYER_CHARACTER;
	botCircleFixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->BOT_CIRCLE;
	centerBoxFixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->PLAYER_CHARACTER;

	topCircleFixture = body->CreateFixture(&topCircleFixtureDef);
	botCircleFixture = body->CreateFixture(&botCircleFixtureDef);
	botCircleFixture->SetUseConstantContactEvenIfNotSensor(true);
	centerBoxFixture = body->CreateFixture(&centerBoxFixtureDef);

	halfBodyHeight = ((b2PolygonShape*)centerBoxFixture->GetShape())->m_vertices[3].y + botCircleShape.m_radius + topCircleShape.m_radius;

	hurt_box = new HurtBox(body, player_index, playerBestiaryData["DictOfUnits"]["Player"]["IdleAnimations"][0]["HurtBoxPerFrame"][0][0]);

	body->SetFixedRotation(true);
	body->SetUserData(this);

	groundCheckShape.SetAsBox(dimensions.x / 8.0f, 0.05f, b2Vec2(0.0f, 0.6f), 0.0f);
	groundCheckFixtureDef.shape = &groundCheckShape;
	groundCheckFixtureDef.isSensor = true;
	groundCheckFixtureDef.m_color = new b2Color(1.0f, 0.0f, 1.0f, 1.0f);

	groundCheckFixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->GROUND_CHECK;
	groundCheckFixtureDef.filter.maskBits = Singleton<SmashWorld>::Get()->PLATFORM;

	body->CreateFixture(&groundCheckFixtureDef);

	numberOfAttacks = (int)Attack::MOVES_COUNT;
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
	attack_input_buffer = new StatusTimer(8);
	jumpInputBuffer = new StatusTimer(6);
	jumpAfterWalkingOffLedgeBuffer = new StatusTimer(4);
	jumpStartUpTimer = new StatusTimer(4);

	weapon = new Weapon(window, position, sf::Vector2f(0.25f, 0.25f), true, player_index, body);// , this);
	
	healthBarRect = new sf::RectangleShape(sf::Vector2f(max_hit_points, 15.0f));
	healthBarRect->setPosition(10.0f, 10.0f);
	healthBarRect->setFillColor(sf::Color::Green);

	healthBarBackgroundRect = new sf::RectangleShape(sf::Vector2f(max_hit_points + 10.0f, 25.0f));
	healthBarBackgroundRect->setPosition(5.0f, 5.0f);
	healthBarBackgroundRect->setFillColor(sf::Color::Black);

	sprite_scale = 1.0f;

	LoadAllAnimations("Player", playerBestiaryData);

	attacks_size = (int)attacks.size();

	int numberOfLandingAnimationFrames = landing_animations.size() > 0 ? landing_animations[0]->GetNumberOfFrames() : 1;
	landing_animation_timer = new StatusTimer(numberOfLandingAnimationFrames);

	is_hittable = true;

	int numberOfDyingAnimationFrames = dying_animations.size() > 0 ? dying_animations[0]->GetNumberOfFrames() : 0;
	dying_animation_timer = new StatusTimer(numberOfDyingAnimationFrames);

	ringbearerFont.loadFromFile("Images/RingbearerFont.ttf");

	sf::Vector2f viewport_dimensions = Singleton<SmashWorld>().Get()->GetCamera()->viewport_dimensions;

	body->SetLinearVelocity(b2Vec2(0.0f, 0.0f)); 

	DropThroughPassThroughPlatforms = false;

	IsSkateboarding = false;
	currentNumberOfSpeedUpsAvailable = maxNumberOfSpeedUpsAvailable = 5;
	timeOfLastSpeedUpUse = 0;
	timeOfLastSpeedUpReplenished = 0;
	timeBeforeReplenishingSpeedUps = 240;
	timeBetweenNextSpeedUpReplenishes = 60;
	timerForMinimumTimeBetweenSpeedUps = new StatusTimer(40);

	speedUpRectColor = sf::Color(255, 128, 0, 255);

	speedUpRects.push_back(new sf::RectangleShape(sf::Vector2f(25.0f, 25.0f)));
	speedUpRects[0]->setFillColor(speedUpRectColor);
	speedUpRects[0]->setPosition(sf::Vector2f(10.0f, 30.0f));
	speedUpRects.push_back(new sf::RectangleShape(sf::Vector2f(25.0f, 25.0f)));
	speedUpRects[1]->setFillColor(speedUpRectColor);
	speedUpRects[1]->setPosition(sf::Vector2f(40.0f, 30.0f));
	speedUpRects.push_back(new sf::RectangleShape(sf::Vector2f(25.0f, 25.0f)));
	speedUpRects[2]->setFillColor(speedUpRectColor);
	speedUpRects[2]->setPosition(sf::Vector2f(70.0f, 30.0f));
	speedUpRects.push_back(new sf::RectangleShape(sf::Vector2f(25.0f, 25.0f)));
	speedUpRects[3]->setFillColor(speedUpRectColor);
	speedUpRects[3]->setPosition(sf::Vector2f(100.0f, 30.0f));
	speedUpRects.push_back(new sf::RectangleShape(sf::Vector2f(25.0f, 25.0f)));
	speedUpRects[4]->setFillColor(speedUpRectColor);
	speedUpRects[4]->setPosition(sf::Vector2f(130.0f, 30.0f));
}

void SmashCharacter::ReceiveHeal(int heal) {
	BoulderCreature::ReceiveHeal(heal);
	UpdateHealthBar();
}

void SmashCharacter::UpdateHealthBar() {
	float percent_health = ((float)hit_points / (float)max_hit_points);

	healthBarRect->setSize(sf::Vector2f(max_hit_points * percent_health, 15.0f));
	healthBarRect->setPosition(10.0f, 10.0f);
	healthBarRect->setFillColor(sf::Color((int)(0.0f + (255.0f - (float)hit_points / (float)max_hit_points) * 255.0f), (int)((float)hit_points / (float)max_hit_points * 255.0f), 0, 255));

	healthBarBackgroundRect->setSize(sf::Vector2f(max_hit_points + 10.0f, 25.0f));
	healthBarBackgroundRect->setPosition(5.0f, 5.0f);
	healthBarBackgroundRect->setFillColor(sf::Color::Black);
}

void SmashCharacter::TakeDamage(int damage, sf::Vector2f knock_back, int hit_stun_frames, bool pop_up_grounded_enemies) {
	bool was_alive = hit_points > 0;

	BoulderCreature::TakeDamage(damage, knock_back, hit_stun_frames, pop_up_grounded_enemies);
	UpdateHealthBar();

	if (was_alive && hit_points <= 0) {
		Singleton<SmashWorld>::Get()->PlayerDied();
	}
}

void SmashCharacter::ApplyObjectDataToSaveData(Json::Value& save_data) {
	BoulderCreature::ApplyObjectDataToSaveData(save_data["Player"]);
	weapon->ApplyObjectDataToSaveData(save_data["Player"]["Weapon"]);

	save_data["Player"]["MaximumHitPoints"] = max_hit_points;
	save_data["Player"]["IsFacingRight"] = IsFacingRight();
}

void SmashCharacter::ApplySaveDataToObjectData(Json::Value& save_data) {
	BoulderCreature::ApplySaveDataToObjectData(save_data["Player"]);
	weapon->ApplySaveDataToObjectData(save_data["Player"]["Weapon"]);

	SetFacingRight(save_data["Player"]["IsFacingRight"].asBool());

	UpdateHealthBar();
}

void SmashCharacter::Update(sf::Int64 curr_frame, sf::Int64 delta_time) {
	Creature::Update(curr_frame, delta_time);

	UpdateSpeedUpValues();

	b2Vec2 body_lin_vel = body->GetLinearVelocity();

	if ((int)platformContacts.size() > 0) {
		b2Vec2 platform_lin_vel = platformContacts[0]->GetBody()->GetLinearVelocity();

		body->SetLinearVelocity(b2Vec2(body_lin_vel.x + platform_lin_vel.x, body_lin_vel.y));
	}

	if (DropThroughPassThroughPlatforms && leftStickInputs.y <= 90.0f) {
		DropThroughPassThroughPlatforms = false;
	}

	if (IsInTheAir()) {
		b2Vec2 vel = body->GetLinearVelocity();

		if (leftStickInputs.y > 90.0f && vel.y > 0) {
			fastFalling = true;
		}

		if (fastFalling) {
			body->SetLinearVelocity(b2Vec2(vel.x, fastFallingVelocity));
			maxVerticalVelocityReached = fastFallingVelocity;
		} else if (vel.y > normalTerminalVelocity) {
			body->SetLinearVelocity(b2Vec2(vel.x, normalTerminalVelocity));
		}
	}
	
	if (jumpStartUpTimerWasActiveLastFrame && !jumpStartUpTimer->IsActive()) {
		ActuallyJump(releasedJumpButton);
	} else if (anAttackWasActiveLastFrame && !IsAnAttackActive() && !landing_animation_timer->IsActive()) {
		if (jumpInputBufferWasActiveLastFrame && !jumpInputBuffer->IsActive()) {
			StartJump();
		} else if (attack_input_buffer->IsActive()) {
			UseAttack(attack_buffer_attack_index, false);
		}
	}

	jumpInputBufferWasActiveLastFrame = jumpInputBuffer->IsActive();
	jumpStartUpTimerWasActiveLastFrame = jumpStartUpTimer->IsActive();
	anAttackWasActiveLastFrame = IsAnAttackActive();


	can_take_input = !hit_stun_timer->IsActive();

	hurt_box->Update(IsFacingRight());

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
	} else if (leftStickInputs.x != 0) {
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
			RightFootStepSound.play();
		} else if (walking_animations[0]->GetCurrentFrame() == LeftFootStepSoundFrameWalk) {
			LeftFootStepSound.play();
		}
	} else if (State == STATE_RUNNING) {
		if (running_animations[0]->GetCurrentFrame() == RightFootStepSoundFrameRun) {
			RightFootStepSound.play();
		} else if (running_animations[0]->GetCurrentFrame() == LeftFootStepSoundFrameRun) {
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

void SmashCharacter::DetermineWhichAttackToUseAndActivateIt(float x_input, float y_input) {
	if (IsInTheAir()) {
		if (y_input < -90.0f) {
			UseAttack(Attack::UP_AIR);
		} else if (y_input > 90.0f) {
			UseAttack(Attack::DOWN_AIR);
		} else if (x_input < -90.0f) {
			if (IsFacingRight()) {
				UseAttack(Attack::BACK_AIR);
			} else {
				UseAttack(Attack::FORWARD_AIR);
			}
		} else if (x_input > 90.0f) {
			if (IsFacingRight()) {
				UseAttack(Attack::FORWARD_AIR);
			} else {
				UseAttack(Attack::BACK_AIR);
			}
		} else {
			UseAttack(Attack::NEUTRAL_AIR);
		}
	} else {
		if (y_input < -90.0f) {
			UseAttack(Attack::UP_SMASH);
		} else if (y_input > 90.0f) {
			UseAttack(Attack::DOWN_SMASH);
		} else if (x_input < -90.0f) {
			if (IsFacingRight() && !IsAnAttackActive()) {
				SetFacingRight(false);
			}
			attacks[Attack::FORWARD_SMASH]->SetFacingRight(IsFacingRight());
			UseAttack(Attack::FORWARD_SMASH);
		} else if (x_input > 90.0f) {
			if (!IsFacingRight() && !IsAnAttackActive()) {
				SetFacingRight(true);
			}
			attacks[Attack::FORWARD_SMASH]->SetFacingRight(IsFacingRight());
			UseAttack(Attack::FORWARD_SMASH);
		} else {
			UseAttack(Attack::JAB);
		}
	}
}

void SmashCharacter::Draw(sf::Vector2f camera_position) {
	render_window->draw(*healthBarBackgroundRect);

	for (int i = 0; i < currentNumberOfSpeedUpsAvailable; ++i) {
		render_window->draw(*speedUpRects[i]);
	}

	BoulderCreature::Draw(camera_position);
}

void SmashCharacter::HandleButtonXPress() {
	if (interactable != nullptr) {
		interactable->StartTalking();
		Singleton<SmashWorld>::Get()->StartDialogue(interactable->GetType());
	} else {
		DetermineWhichAttackToUseAndActivateIt(leftStickInputs.x, leftStickInputs.y);
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
	if (weapon->CanTeleportToWeapon() && !teleportedSinceLastLanding) {
		if (IsInTheAir()) {
			teleportedSinceLastLanding = true;
		}

		b2Vec2 original_position = body->GetPosition();
		b2Vec2 new_position = weapon->GetBody()->GetPosition();

		b2Vec2 travel_vector = b2Vec2(new_position.x - original_position.x, new_position.y - original_position.y);
		travel_vector.Normalize();
		float32 speed = 10.0f;

		body->SetTransform(weapon->GetBody()->GetPosition(), body->GetAngle());
		body->SetLinearVelocity(b2Vec2(travel_vector.x * speed, travel_vector.y * speed));

		weapon->TeleportedToWeapon();
	}
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

void SmashCharacter::ForcedRecall() {
	weapon->ForcedRecall();
}

void SmashCharacter::Land() {
	teleportedSinceLastLanding = false;

	BoulderCreature::Land();
}

int SmashCharacter::GetDamageOfCurrentAttack() {
	Attack* active_attack = GetActiveAttack();

	if (active_attack != nullptr) {
		int damage = active_attack->GetDamage();

		return damage;
	}

	return 0;
}

void SmashCharacter::UpdateEffectsVolumes(float new_effects_volume) {
	BoulderCreature::UpdateEffectsVolumes(new_effects_volume);

	for (int i = 0; i < (int)AttackAnimationSounds.size(); i++) {
		AttackAnimationSounds[i]->setVolume(new_effects_volume);
	}
}

void SmashCharacter::HandleDpadRightPress() {
}

void SmashCharacter::HandleDpadRightRelease() {
}

void SmashCharacter::HandleDpadLeftPress() {
}

void SmashCharacter::HandleDpadLeftRelease() {
}

void SmashCharacter::HandleDpadUpPress() {
}

void SmashCharacter::HandleDpadUpRelease() {
}

void SmashCharacter::HandleDpadDownPress() {
}

void SmashCharacter::HandleDpadDownRelease() {
}

void SmashCharacter::ActuallyJump(bool short_hop) {
	if (can_take_input && hit_points > 0 && !IsAnAttackActive() && !landing_animation_timer->IsActive()) {
		bool jumping = false;

		if (!IsInTheAir() || jumpAfterWalkingOffLedgeBuffer->IsActive()) {
			jumping = true;
			jumpAfterWalkingOffLedgeBuffer->Stop();
		} else if (has_double_jump) {
			jumping = true;
			has_double_jump = false;
		}

		if (jumping) {
			maxAirSpeed = body->GetLinearVelocity().x;

			body->SetLinearVelocity(b2Vec2(maxAirSpeed, short_hop ? -jump_power * 0.8f : -jump_power));
			SetInTheAir(true);
		}
	}
}

void SmashCharacter::UpdateSpeedUpValues() {
	if (currentNumberOfSpeedUpsAvailable < maxNumberOfSpeedUpsAvailable) {
		if (timeOfLastSpeedUpUse + timeBeforeReplenishingSpeedUps < current_frame && timeOfLastSpeedUpReplenished + timeBetweenNextSpeedUpReplenishes < current_frame) {
			currentNumberOfSpeedUpsAvailable++;
			timeOfLastSpeedUpReplenished = current_frame;
		}
	}
}

void SmashCharacter::HandleLeftStickInput(float horizontal, float vertical) {
	if (!IsSkateboarding || IsInTheAir()) {
		Move(horizontal, vertical);
	}

	leftStickInputs.x = horizontal;
	leftStickInputs.y = vertical;
}

void SmashCharacter::HandleRightStickInput(float horizontal, float vertical) {
	if (vertical > -90.0f && vertical < 90.0f && horizontal > -90.0f && horizontal < 90.0f) {
		rightStickWasCentered = true;
		return;
	}

	if (!rightStickWasCentered) {
		return;
	}

	rightStickWasCentered = false;

	DetermineWhichAttackToUseAndActivateIt(horizontal, vertical);
}

void SmashCharacter::HandleButtonAPress() {
	releasedJumpButton = false;

	if (IsInTheAir() && !has_double_jump/* && IsAnAttackActive()*/) {
		jumpInputBuffer->Start();
	}
	else if (leftStickInputs.y > 90.0f) {
		DropThroughPassThroughPlatforms = true;
	}
	else {
		StartJump();
	}
}

void SmashCharacter::HandleButtonARelease() {
	releasedJumpButton = true;
}

void SmashCharacter::HandleButtonBPress() {
	weapon->Recall();
}

void SmashCharacter::HandleButtonBRelease() {
}

void SmashCharacter::HandleButtonXRelease() {
}

void SmashCharacter::HandleButtonYPress() {
	if (weapon->Throwable()) {
		ThrowWeapon();
	}
	else {
		TeleportToWeapon();
	}
}

void SmashCharacter::HandleButtonYRelease() {
}

void SmashCharacter::HandleButtonRightBumperPress() {
	IsSkateboarding = true;
	botCircleFixture->SetFriction(0.0f);
	centerBoxFixture->SetFriction(0.0f);

	Singleton<SmashWorld>::Get()->StartedUsingSkateBoard();
}

void SmashCharacter::HandleButtonRightBumperRelease() {
	IsSkateboarding = false;
	botCircleFixture->SetFriction(1.0f);
	centerBoxFixture->SetFriction(1.0f);

	Singleton<SmashWorld>::Get()->PutAwaySkateBoard();
}

void SmashCharacter::HandleButtonLeftBumperPress() {
	if (IsSkateboarding && currentNumberOfSpeedUpsAvailable > 0 && !IsInTheAir() && !timerForMinimumTimeBetweenSpeedUps->IsActive()) {
		currentNumberOfSpeedUpsAvailable--;
		timeOfLastSpeedUpUse = current_frame;
		timerForMinimumTimeBetweenSpeedUps->Start();

		b2Vec2 vel = body->GetLinearVelocity();
		body->SetLinearVelocity(b2Vec2(vel.x * 1.5f, vel.y * 1.5f));
	}
	//running = true;
}

void SmashCharacter::HandleButtonLeftBumperRelease() {
	//running = false;
}