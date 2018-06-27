#pragma once

#ifndef SMASH_CHARACTER_H
#define SMASH_CHARACTER_H

using namespace std;
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include "GameLibrary\SpriteAnimation.h"
#include "GameLibrary\Settings.h"
#include "BoulderCreature.h"
#include "Projectile.h"
#include "PlatformerLibrary\Box2D\Box2D.h"
#include "ControllableCharacter.h"
#include "Attack.h"
#include "Weapon.h"
#include "StatusTimer.h"
#include "Rune.h"
#include "GameLibrary\Json\json.h"

class SmashCharacter : public BoulderCreature, public ControllableCharacter {
private:
	int numberOfAttacks;
	Weapon* weapon;
	void DashPunch();
	void ThrowWeapon();
	void TeleportToWeapon();

	sf::RectangleShape* healthBarBackgroundRect;
	bool teleportedSinceLastLanding;
	bool rightStickWasCentered;
	sf::Vector2f leftStickInputs;
	sf::Font ringbearerFont;
	float halfBodyHeight;

	/* SKATEBOARDING VARIABLES */
	int maxNumberOfSpeedUpsAvailable;
	int currentNumberOfSpeedUpsAvailable;
	sf::Int64 timeOfLastSpeedUpUse;
	sf::Int64 timeOfLastSpeedUpReplenished;
	sf::Int64 timeBeforeReplenishingSpeedUps;
	sf::Int64 timeBetweenNextSpeedUpReplenishes;
	StatusTimer* timerForMinimumTimeBetweenSpeedUps;
	sf::Color speedUpRectColor;
	std::vector<sf::RectangleShape*> speedUpRects;
protected:
	virtual void StartJump();
	virtual void ActuallyJump(bool short_hop = false);
	virtual void DrawAnimationsBasedOnState(sf::Vector2f camera_position);

	const int STATE_SKATING = 100;
	const int STATE_SKATE_KICKING = 101;
public:
	SmashCharacter(int player_idx, Json::Value playerBestiaryData, sf::RenderWindow *window, sf::Vector2f position = sf::Vector2f(0.0f, 0.0f), sf::Vector2f dimensions = sf::Vector2f(0.0f, 0.0f), bool subject_to_gravity = true);
	virtual void Draw(sf::Vector2f camera_position);
	virtual void Update(sf::Int64 curr_frame, sf::Int64 delta_time);
	virtual void ApplyObjectDataToSaveData(Json::Value& save_data);
	virtual void ApplySaveDataToObjectData(Json::Value& save_data);
	virtual void TakeDamage(int damage, sf::Vector2f knock_back, int hit_stun_frames, bool pop_up_grounded_enemies);
	virtual void ReceiveHeal(int heal);
	virtual void Land();
	virtual void UpdateEffectsVolumes(float new_effects_volume);
	virtual void AddPlatformContact(Box2DRigidBody* platform);
	virtual void RemovePlatformContact(Box2DRigidBody* platform);
	virtual int GetDamageOfCurrentAttack();
	void UpdateHealthBar();
	void ForcedRecall();
	void DetermineWhichAttackToUseAndActivateIt(float x_input, float y_input);
	void UpdateSpeedUpValues();

	virtual int GetPlayerIndex() {
		return player_index;
	};
	virtual void SetPlayerIndex(int id) {
		player_index = id;
	};
	virtual int GetHitPoints() {
		return hit_points;
	};
	virtual int GetMaxHitPoints() {
		return max_hit_points;
	};
	virtual bool CanTakeInput() {
		return can_take_input;
	};
	
	void SmashCharacter::HandleLeftStickInput(float horizontal, float vertical);

	void SmashCharacter::HandleRightStickInput(float horizontal, float vertical);

	void SmashCharacter::HandleButtonAPress();

	void SmashCharacter::HandleButtonARelease();

	void SmashCharacter::HandleButtonBPress();

	void SmashCharacter::HandleButtonBRelease();

	void SmashCharacter::HandleButtonXPress();

	void SmashCharacter::HandleButtonXRelease();

	void SmashCharacter::HandleButtonYPress();

	void SmashCharacter::HandleButtonYRelease();

	void SmashCharacter::HandleButtonRightBumperPress();

	void SmashCharacter::HandleButtonRightBumperRelease();

	void SmashCharacter::HandleButtonLeftBumperPress();

	void SmashCharacter::HandleButtonLeftBumperRelease();

	void SmashCharacter::HandleButtonSelectPress();

	void SmashCharacter::HandleButtonSelectRelease();

	void SmashCharacter::HandleButtonStartPress();

	void SmashCharacter::HandleButtonStartRelease();

	void SmashCharacter::HandleDpadRightPress();

	void SmashCharacter::HandleDpadRightRelease();

	void SmashCharacter::HandleDpadLeftPress();

	void SmashCharacter::HandleDpadLeftRelease();

	void SmashCharacter::HandleDpadUpPress();

	void SmashCharacter::HandleDpadUpRelease();

	void SmashCharacter::HandleDpadDownPress();

	void SmashCharacter::HandleDpadDownRelease();
};

#endif