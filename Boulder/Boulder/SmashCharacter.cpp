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

	characterLevel = 1;
	weaponLevel = 1;
	characterLevelForDisplay = 1;
	weaponLevelForDisplay = 1;
	characterExperienceTowardsNextLevel = 0;
	weaponExperienceTowardsNextLevel = 0;
	characterExperienceTowardsNextLevelForAnimatedBar = 0;
	weaponExperienceTowardsNextLevelForAnimatedBar = 0;

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
	botCircleFixtureDef.friction = 0.3f;
	botCircleFixtureDef.m_color = new b2Color(0.0f, 1.0f, 0.0f, 1.0f);

	topCircleFixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->PLAYER_CHARACTER;
	botCircleFixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->PLAYER_CHARACTER;
	centerBoxFixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->PLAYER_CHARACTER;

	topCircleFixture = body->CreateFixture(&topCircleFixtureDef);
	botCircleFixture = body->CreateFixture(&botCircleFixtureDef);
	centerBoxFixture = body->CreateFixture(&centerBoxFixtureDef);

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
	attack_input_buffer = new StatusTimer(8);
	jumpInputBuffer = new StatusTimer(6);
	jumpStartUpTimer = new StatusTimer(4);

	weapon = new Weapon(window, position, sf::Vector2f(0.25f, 0.25f), true, player_index, body);// , this);
	
	healthBarRect = new sf::RectangleShape(sf::Vector2f(max_hit_points, 15.0f));
	healthBarRect->setPosition(10.0f, 10.0f);
	healthBarRect->setFillColor(sf::Color::Green);

	healthBarBackgroundRect = new sf::RectangleShape(sf::Vector2f(max_hit_points + 10.0f, 25.0f));
	healthBarBackgroundRect->setPosition(5.0f, 5.0f);
	healthBarBackgroundRect->setFillColor(sf::Color::Black);

	UpdateCharacterExperienceBar();
	UpdateWeaponExperienceBar();

	sprite_scale = 1.0f;

	LoadAllAnimations("Player", playerBestiaryData);

	attacks_size = (int)attacks.size();

	int numberOfLandingAnimationFrames = landing_animations.size() > 0 ? landing_animations[0]->GetNumberOfFrames() : 1;
	landing_animation_timer = new StatusTimer(numberOfLandingAnimationFrames);

	is_hittable = true;

	int numberOfDyingAnimationFrames = dying_animations.size() > 0 ? dying_animations[0]->GetNumberOfFrames() : 0;
	dying_animation_timer = new StatusTimer(numberOfDyingAnimationFrames);

	ringbearerFont.loadFromFile("Images/RingbearerFont.ttf");
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

void SmashCharacter::UpdateCharacterExperienceBar() {
	bool leveling_up = characterLevelForDisplay < characterLevel;
	int animated_bar_goal = characterExperienceTowardsNextLevel;
	float bar_x = (characterLevelForDisplay >= 10 || weaponLevelForDisplay >= 10 ? 40.0f : 25.0f);

	if (leveling_up) {
		animated_bar_goal = CharacterExperienceNeededForNextLevel(characterLevelForDisplay);
	}

	if (characterExperienceTowardsNextLevelForAnimatedBar < animated_bar_goal) {
		if (leveling_up) {
			characterExperienceTowardsNextLevelForAnimatedBar += (characterLevel - characterLevelForDisplay) + 1;
		} else {
			characterExperienceTowardsNextLevelForAnimatedBar += 1;
		}
	}
	
	if (leveling_up && characterExperienceTowardsNextLevelForAnimatedBar >= animated_bar_goal) {
		characterLevelForDisplay++;
		characterExperienceTowardsNextLevelForAnimatedBar = 0;
		leveling_up = characterLevelForDisplay < characterLevel;

		if (leveling_up) {
			animated_bar_goal = CharacterExperienceNeededForNextLevel(characterLevelForDisplay);
		}
	}

	if (characterExperienceBarRect == nullptr) {
		characterExperienceBarRect = new sf::RectangleShape(sf::Vector2f((float)animated_bar_goal, 15.0f));
		characterExperienceBarRect->setPosition(bar_x, 40.0f);
		characterExperienceBarRect->setFillColor(sf::Color(255, 153, 153, 255));
	} else {
		characterExperienceBarRect->setSize(sf::Vector2f((float)animated_bar_goal, 15.0f));
		characterExperienceBarRect->setPosition(bar_x, 40.0f);
	}

	if (characterExperienceBarAnimatedRect == nullptr) {
		characterExperienceBarAnimatedRect = new sf::RectangleShape(sf::Vector2f((float)characterExperienceTowardsNextLevelForAnimatedBar, 15.0f));
		characterExperienceBarAnimatedRect->setPosition(bar_x, 40.0f);
		characterExperienceBarAnimatedRect->setFillColor(sf::Color::Red);
	} else {
		characterExperienceBarAnimatedRect->setSize(sf::Vector2f((float)characterExperienceTowardsNextLevelForAnimatedBar, 15.0f));
		characterExperienceBarAnimatedRect->setPosition(bar_x, 40.0f);
	}

	if (characterExperienceBarBackgroundRect == nullptr) {
		characterExperienceBarBackgroundRect = new sf::RectangleShape(sf::Vector2f((float)CharacterExperienceNeededForNextLevel(characterLevelForDisplay), 15.0f));
		characterExperienceBarBackgroundRect->setPosition(bar_x, 40.0f);
		characterExperienceBarBackgroundRect->setFillColor(sf::Color::White);
		characterExperienceBarBackgroundRect->setOutlineThickness(5.0f);
		characterExperienceBarBackgroundRect->setOutlineColor(sf::Color::Black);
	} else {
		characterExperienceBarBackgroundRect->setSize(sf::Vector2f((float)CharacterExperienceNeededForNextLevel(characterLevelForDisplay), 15.0f));
		characterExperienceBarBackgroundRect->setPosition(bar_x, 40.0f);
	}

	if (characterLevelText == nullptr) {
		characterLevelText = new sf::Text(to_string(characterLevelForDisplay), ringbearerFont, 25);
		characterLevelText->setFillColor(sf::Color::White);
		characterLevelText->setOutlineColor(sf::Color::Black);
		characterLevelText->setOutlineThickness(1.5f);
		characterLevelText->setPosition(5.0f, 30.0f);
	} else if (characterLevelText->getString() != to_string(characterLevelForDisplay)) {
		characterLevelText->setString(to_string(characterLevelForDisplay));
	}
}

void SmashCharacter::UpdateWeaponExperienceBar() {
	bool leveling_up = weaponLevelForDisplay < weaponLevel;
	int animated_bar_goal = weaponExperienceTowardsNextLevel;
	float bar_x = (characterLevelForDisplay >= 10 || weaponLevelForDisplay >= 10 ? 40.0f : 25.0f);

	if (leveling_up) {
		animated_bar_goal = WeaponExperienceNeededForNextLevel(weaponLevelForDisplay);
	}

	if (weaponExperienceTowardsNextLevelForAnimatedBar < animated_bar_goal) {
		if (leveling_up) {
			weaponExperienceTowardsNextLevelForAnimatedBar += (weaponLevel - weaponLevelForDisplay) + 1;
		}
		else {
			weaponExperienceTowardsNextLevelForAnimatedBar += 1;
		}
	}

	if (leveling_up && weaponExperienceTowardsNextLevelForAnimatedBar >= animated_bar_goal) {
		weaponLevelForDisplay++;
		weaponExperienceTowardsNextLevelForAnimatedBar = 0;
		leveling_up = weaponLevelForDisplay < weaponLevel;

		if (leveling_up) {
			animated_bar_goal = WeaponExperienceNeededForNextLevel(weaponLevelForDisplay);
		}
	}

	if (weaponExperienceBarRect == nullptr) {
		weaponExperienceBarRect = new sf::RectangleShape(sf::Vector2f((float)animated_bar_goal, 15.0f));
		weaponExperienceBarRect->setPosition(bar_x, 70.0f);
		weaponExperienceBarRect->setFillColor(sf::Color(153, 153, 255, 255));
	}
	else {
		weaponExperienceBarRect->setSize(sf::Vector2f((float)animated_bar_goal, 15.0f));
		weaponExperienceBarRect->setPosition(bar_x, 70.0f);
	}

	if (weaponExperienceBarAnimatedRect == nullptr) {
		weaponExperienceBarAnimatedRect = new sf::RectangleShape(sf::Vector2f((float)weaponExperienceTowardsNextLevelForAnimatedBar, 15.0f));
		weaponExperienceBarAnimatedRect->setPosition(bar_x, 70.0f);
		weaponExperienceBarAnimatedRect->setFillColor(sf::Color::Blue);
	}
	else {
		weaponExperienceBarAnimatedRect->setSize(sf::Vector2f((float)weaponExperienceTowardsNextLevelForAnimatedBar, 15.0f));
		weaponExperienceBarAnimatedRect->setPosition(bar_x, 70.0f);
	}

	if (weaponExperienceBarBackgroundRect == nullptr) {
		weaponExperienceBarBackgroundRect = new sf::RectangleShape(sf::Vector2f((float)WeaponExperienceNeededForNextLevel(weaponLevelForDisplay), 15.0f));
		weaponExperienceBarBackgroundRect->setPosition(bar_x, 70.0f);
		weaponExperienceBarBackgroundRect->setFillColor(sf::Color::White);
		weaponExperienceBarBackgroundRect->setOutlineThickness(5.0f);
		weaponExperienceBarBackgroundRect->setOutlineColor(sf::Color::Black);
	}
	else {
		weaponExperienceBarBackgroundRect->setSize(sf::Vector2f((float)WeaponExperienceNeededForNextLevel(weaponLevelForDisplay), 15.0f));
		weaponExperienceBarBackgroundRect->setPosition(bar_x, 70.0f);
	}

	if (weaponLevelText == nullptr) {
		weaponLevelText = new sf::Text(to_string(weaponLevelForDisplay), ringbearerFont, 25);
		weaponLevelText->setFillColor(sf::Color::White);
		weaponLevelText->setOutlineColor(sf::Color::Black);
		weaponLevelText->setOutlineThickness(1.5f);
		weaponLevelText->setPosition(5.0f, 60.0f);
	}
	else if (weaponLevelText->getString() != to_string(weaponLevelForDisplay)) {
		weaponLevelText->setString(to_string(weaponLevelForDisplay));
	}
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

	save_data["Player"]["MaximumHitPoints"] = max_hit_points;
	save_data["Player"]["CharacterLevel"] = characterLevel;
	save_data["Player"]["WeaponLevel"] = weaponLevel;
	save_data["Player"]["CharacterExperienceTowardsNextLevel"] = characterExperienceTowardsNextLevel;
	save_data["Player"]["WeaponExperienceTowardsNextLevel"] = weaponExperienceTowardsNextLevel;
}

void SmashCharacter::ApplySaveDataToObjectData(Json::Value& save_data) {
	BoulderCreature::ApplySaveDataToObjectData(save_data["Player"]);

	max_hit_points = save_data["Player"]["MaximumHitPoints"].asInt();
	characterLevelForDisplay = characterLevel = save_data["Player"]["CharacterLevel"].asInt();
	weaponLevelForDisplay = weaponLevel = save_data["Player"]["WeaponLevel"].asInt();
	characterExperienceTowardsNextLevel = save_data["Player"]["CharacterExperienceTowardsNextLevel"].asInt();
	weaponExperienceTowardsNextLevel = save_data["Player"]["WeaponExperienceTowardsNextLevel"].asInt();

	UpdateHealthBar();
	UpdateCharacterExperienceBar();
	UpdateWeaponExperienceBar();
}

void SmashCharacter::Update(sf::Int64 curr_frame, sf::Int64 delta_time) {
	Creature::Update(curr_frame, delta_time);

	UpdateCharacterExperienceBar();
	UpdateWeaponExperienceBar();

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

	//cout << body->GetLinearVelocity().y << "\r";

	can_take_input = !hit_stun_timer->IsActive();
	//if (can_take_input) {
	//	can_take_input = !IsAnAttackActive();
	//}

	hurt_box->Update(IsFacingRight());

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
			//RightFootStepSound.setVolume(25);
			RightFootStepSound.play();
		} else if (walking_animations[0]->GetCurrentFrame() == LeftFootStepSoundFrameWalk) {
			//LeftFootStepSound.setVolume(25);
			LeftFootStepSound.play();
		}
	} else if (State == STATE_RUNNING) {
		if (running_animations[0]->GetCurrentFrame() == RightFootStepSoundFrameRun) {
			//RightFootStepSound.setVolume(60);
			RightFootStepSound.play();
		} else if (running_animations[0]->GetCurrentFrame() == LeftFootStepSoundFrameRun) {
			//LeftFootStepSound.setVolume(60);
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
			//cout << "UP_AIR\n";
		} else if (y_input > 90.0f) {
			UseAttack(Attack::DOWN_AIR);
			//cout << "DOWN_AIR\n";
		} else if (x_input < -90.0f) {
			if (IsFacingRight()) {
				UseAttack(Attack::BACK_AIR);
				//cout << "BACK_AIR\n";
			} else {
				UseAttack(Attack::FORWARD_AIR);
				//cout << "FORWARD_AIR\n";
			}
		} else if (x_input > 90.0f) {
			if (IsFacingRight()) {
				UseAttack(Attack::FORWARD_AIR);
				//cout << "FORWARD_AIR\n";
			} else {
				UseAttack(Attack::BACK_AIR);
				//cout << "BACK_AIR\n";
			}
		} else {
			UseAttack(Attack::NEUTRAL_AIR);
		}
	} else {
		if (y_input < -90.0f) {
			UseAttack(Attack::UP_SMASH);
			//cout << "UP_SMASH\n";
		} else if (y_input > 90.0f) {
			UseAttack(Attack::DOWN_SMASH);
			//cout << "DOWN_SMASH\n";
		} else if (x_input < -90.0f) {
			if (IsFacingRight()) {
				SetFacingRight(false);
			}
			attacks[Attack::FORWARD_SMASH]->Update(current_frame, IsFacingRight());
			UseAttack(Attack::FORWARD_SMASH);
			//cout << "FORWARD_SMASH\n";
		} else if (x_input > 90.0f) {
			if (!IsFacingRight()) {
				SetFacingRight(true);
			}
			attacks[Attack::FORWARD_SMASH]->Update(current_frame, IsFacingRight());
			UseAttack(Attack::FORWARD_SMASH);
			//cout << "FORWARD_SMASH\n";
		} else {
			UseAttack(Attack::JAB);
		}
	}
}

void SmashCharacter::Draw(sf::Vector2f camera_position) {
	render_window->draw(*healthBarBackgroundRect);
	render_window->draw(*characterExperienceBarBackgroundRect);
	render_window->draw(*characterExperienceBarRect); 
	render_window->draw(*characterExperienceBarAnimatedRect);
	render_window->draw(*weaponExperienceBarBackgroundRect);
	render_window->draw(*weaponExperienceBarRect);
	render_window->draw(*weaponExperienceBarAnimatedRect);

	render_window->draw(*characterLevelText);
	render_window->draw(*weaponLevelText);

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
		b2Vec2 original_position = body->GetPosition();
		b2Vec2 new_position = weapon->GetBody()->GetPosition();

		b2Vec2 travel_vector = b2Vec2(new_position.x - original_position.x, new_position.y - original_position.y);
		travel_vector.Normalize();
		float32 speed = 10.0f;

		body->SetTransform(weapon->GetBody()->GetPosition(), body->GetAngle());
		body->SetLinearVelocity(b2Vec2(travel_vector.x * speed, travel_vector.y * speed));

		teleportedSinceLastLanding = true;

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

int SmashCharacter::CharacterExperienceNeededForNextLevel(int level) {
	return 100 + (20 * level);
}

int SmashCharacter::WeaponExperienceNeededForNextLevel(int level) {
	return 100 + (10 * level);
}

void SmashCharacter::ReceiveExperience(int experience_points) {
	characterExperienceTowardsNextLevel += experience_points;
	weaponExperienceTowardsNextLevel += experience_points;

	while (characterExperienceTowardsNextLevel >= CharacterExperienceNeededForNextLevel(characterLevel)) {
		LevelUpCharacter();
	}

	while (weaponExperienceTowardsNextLevel >= WeaponExperienceNeededForNextLevel(weaponLevel)) {
		LevelUpWeapon();
	}

	UpdateCharacterExperienceBar();
	UpdateWeaponExperienceBar();
}

void SmashCharacter::LevelUpCharacter() {
	characterExperienceTowardsNextLevel -= CharacterExperienceNeededForNextLevel(characterLevel);
	characterLevel++;

	max_hit_points += 15;
	hit_points = max_hit_points;

	healthBarRect = new sf::RectangleShape(sf::Vector2f(hit_points, 15.0f));
	healthBarRect->setPosition(10.0f, 10.0f);
	healthBarRect->setFillColor(sf::Color::Green);

	healthBarBackgroundRect = new sf::RectangleShape(sf::Vector2f(hit_points + 10.0f, 25.0f));
	healthBarBackgroundRect->setPosition(5.0f, 5.0f);
	healthBarBackgroundRect->setFillColor(sf::Color::Black);
}

void SmashCharacter::LevelUpWeapon() {
	weaponExperienceTowardsNextLevel -= WeaponExperienceNeededForNextLevel(weaponLevel);
	weaponLevel++;
}

int SmashCharacter::GetDamageOfCurrentAttack() {
	return (int)(GetActiveAttack()->GetDamage() * (1.0f + weaponLevel * 0.1f));
}

void SmashCharacter::UpdateEffectsVolumes(float new_effects_volume) {
	BoulderCreature::UpdateEffectsVolumes(new_effects_volume);

	for (int i = 0; i < (int)AttackAnimationSounds.size(); i++) {
		AttackAnimationSounds[i]->setVolume(new_effects_volume);
	}
}