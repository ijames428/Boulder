#pragma once

#ifndef SMASH_CHARACTER_H
#define SMASH_CHARACTER_H

using namespace std;
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
//#include "..\PlatformerLibrary\RigidBody.h"
#include "GameLibrary\SpriteAnimation.h"
#include "GameLibrary\Settings.h"
//#include "..\GameLibrary\AssetManager.h"
//#include "World.h"
#include "BoulderCreature.h"
#include "Projectile.h"
#include "PlatformerLibrary\Box2D\Box2D.h"
#include "ControllableCharacter.h"
#include "Attack.h"
#include "Weapon.h"
#include "StatusTimer.h"
#include "AdvancedInput.h"
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
	int characterLevel;
	//int weaponLevel;
	int characterLevelForDisplay;
	//int weaponLevelForDisplay;
	int characterExperienceTowardsNextLevel;
	int characterExperienceTowardsNextLevelForAnimatedBar;
	//int weaponExperienceTowardsNextLevel;
	//int weaponExperienceTowardsNextLevelForAnimatedBar;
	sf::RectangleShape* characterExperienceBarRect;
	//sf::RectangleShape* weaponExperienceBarRect;
	sf::RectangleShape* characterExperienceBarAnimatedRect;
	//sf::RectangleShape* weaponExperienceBarAnimatedRect;
	sf::RectangleShape* characterExperienceBarBackgroundRect;
	//sf::RectangleShape* weaponExperienceBarBackgroundRect;
	void UpdateCharacterExperienceBar();
	//void UpdateWeaponExperienceBar();
	sf::Font ringbearerFont;
	sf::Text* characterLevelText;
	//sf::Text* weaponLevelText;

	int rageLevel;
	int angerTowardsNextRageLevel;
	int angerNeededForNextRageLevel;
	sf::Text* rageLevelText;
	sf::RectangleShape* rageLevelProgressBarRect;
	sf::RectangleShape* rageLevelBarBackgroundRect;
	std::vector<sf::Color> tierRageColors; 
	void UpdateRageLevelBar();
	void IncreaseRageLevel();
	void DecreaseRageLevel();
	void UpdateRuneUiItems();

	//int numberOfRunesYouCanActivate;

	std::vector<Rune*> tierActivatedRunes;

	sf::Texture* DpadTexture;
	sf::Sprite* DpadSprite;
	float readyRuneOffset = 0.0f;

	sf::Vector2f rune_scale = sf::Vector2f(0.5f, 0.5f);

	float orbitPeriod1 = 0.0f;
	float orbitPeriod2 = 0.0f;
	float orbitPeriod3 = 0.0f;

	Rune* DamageRune;
	Rune* LifestealRune;
	Rune* SuperJumpRune;
	Rune* DefenseRune;
	Rune* BerserkerRune;

	std::vector<SpriteAnimation*> rageTierAuraAnimations;

	int baseMaxHitPoints;
protected:
	virtual void ActuallyJump(bool short_hop = false);
public:
	SmashCharacter(int player_idx, Json::Value playerBestiaryData, sf::RenderWindow *window, sf::Vector2f position = sf::Vector2f(0.0f, 0.0f), sf::Vector2f dimensions = sf::Vector2f(0.0f, 0.0f), bool subject_to_gravity = true);
	void Draw(sf::Vector2f camera_position);
	virtual void Update(sf::Int64 curr_frame, sf::Int64 delta_time);
	virtual void ApplyObjectDataToSaveData(Json::Value& save_data);
	virtual void ApplySaveDataToObjectData(Json::Value& save_data);
	virtual void TakeDamage(int damage, sf::Vector2f knock_back, int hit_stun_frames, bool pop_up_grounded_enemies);
	virtual void ReceiveHeal(int heal);
	void UpdateHealthBar();
	void ForcedRecall();
	virtual void Land();
	void DetermineWhichAttackToUseAndActivateIt(float x_input, float y_input);
	void ReceiveExperience(int experience_points);
	void LevelUpCharacter();
	//void LevelUpWeapon();
	virtual int GetDamageOfCurrentAttack();
	virtual void UpdateEffectsVolumes(float new_effects_volume);
	virtual void AddAnger(int anger_amount);
	int CharacterExperienceNeededForNextLevel(int level);
	//int WeaponExperienceNeededForNextLevel(int level);
	void ResetRuneUiPositions(sf::Vector2f viewport_dimensions);
	float GetDamageMultiplierFromLevel(int char_level);
	int GetNumberOfRuneSlotsFromLevel(int char_level);
	int GetMaximumHitPointsFromLevel(int char_level);

	Rune* DpadLeftRune;
	Rune* DpadUpRune;
	Rune* DpadRightRune;

	std::vector<Rune*> RunesList;

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

	int GetCharacterLevel() {
		return characterLevel;
	};
	//int GetWeaponLevel() {
	//	return weaponLevel;
	//};
	int GetExperienceTowardsNextCharacterLevel() {
		return characterExperienceTowardsNextLevel;
	};
	//int GetExperienceTowardsNextWeaponLevel() {
	//	return weaponExperienceTowardsNextLevel;
	//};
	
	void SmashCharacter::HandleLeftStickInput(float horizontal, float vertical) {
		Move(horizontal, vertical);

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
		} else {
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

	void SmashCharacter::HandleButtonXPress();

	void SmashCharacter::HandleButtonXRelease() {
	}

	void SmashCharacter::HandleButtonYPress() {
		//if (advanced_input->DidPlayerDoQuarterCircleForward(IsFacingRight())) {
		if (weapon->Throwable()) {
			ThrowWeapon();
		}
	}

	void SmashCharacter::HandleButtonYRelease() {
	}

	void SmashCharacter::HandleButtonRightBumperPress() {
		if (!weapon->Throwable()) {
			TeleportToWeapon();
		}
		//TeleportToWeapon();
	}

	void SmashCharacter::HandleButtonRightBumperRelease() {
	}

	void SmashCharacter::HandleButtonLeftBumperPress() {
		running = true;
	}

	void SmashCharacter::HandleButtonLeftBumperRelease() {
		running = false;
	}

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