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

class SmashCharacter : public BoulderCreature, public ControllableCharacter {
private:
	//bool can_take_input;
	Weapon* weapon;
	void DashPunch();
	void ThrowWeapon();
	void TeleportToWeapon();
protected:
public:
	SmashCharacter(int player_idx, sf::RenderWindow *window, sf::Vector2f position = sf::Vector2f(0.0f, 0.0f), sf::Vector2f dimensions = sf::Vector2f(0.0f, 0.0f), bool subject_to_gravity = true);
	//void Draw(sf::Vector2f camera_position);
	virtual void Update(sf::Int64 curr_frame, sf::Int64 delta_time);
	//void TakeDamage(int damage, sf::Vector2f knock_back, int hit_stun_frames);
	//Attack* GetActiveAttack();
	//bool IsAnAttackActive();
	//void Land();

	//string GetName() {
	//	return name;
	//};
	//void SetName(string new_name) {
	//	name = new_name;
	//};
	//b2Body* GetBody() {
	//	return body;
	//}
	virtual int GetPlayerIndex() {
		return player_index;
	};
	virtual void SetPlayerIndex(int id) {
		player_index = id;
	};
	virtual int GetHitPoints() {
		return hit_points;
	};
	virtual bool CanTakeInput() {
		return can_take_input;
	};
	
	void SmashCharacter::HandleLeftStickInput(float horizontal, float vertical) {
		//advanced_input->AddStickInput(horizontal, vertical);

		Move(horizontal, vertical);
	}

	void SmashCharacter::HandleRightStickInput(float horizontal, float vertical) {
		if ((vertical > -0.9f && vertical < 0.9f && horizontal > -0.9f && horizontal < 0.9f) || IsAnAttackActive()) {
			return;
		}

		if (IsInTheAir()) {
			if (vertical < -90.0f) {
				UseAttack(Attack::UP_AIR);
				//cout << "UP_AIR\n";
			} else if (vertical > 90.0f) {
				UseAttack(Attack::DOWN_AIR);
				//cout << "DOWN_AIR\n";
			} else if (horizontal < -90.0f) {
				if (IsFacingRight()) {
					UseAttack(Attack::BACK_AIR);
					//cout << "BACK_AIR\n";
				} else {
					UseAttack(Attack::FORWARD_AIR);
					//cout << "FORWARD_AIR\n";
				}
			} else if (horizontal > 90.0f) {
				if (IsFacingRight()) {
					UseAttack(Attack::FORWARD_AIR);
					//cout << "FORWARD_AIR\n";
				} else {
					UseAttack(Attack::BACK_AIR);
					//cout << "BACK_AIR\n";
				}
			}
		} else {
			if (vertical < -90.0f) {
				UseAttack(Attack::UP_SMASH);
				//cout << "UP_SMASH\n";
			} else if (vertical > 90.0f) {
				UseAttack(Attack::DOWN_SMASH);
				//cout << "DOWN_SMASH\n";
			}else if (horizontal < -90.0f) {
				if (IsFacingRight()) {
					SetFacingRight(false);
				}
				attacks[Attack::FORWARD_SMASH]->Update(current_frame, IsFacingRight());
				UseAttack(Attack::FORWARD_SMASH);
				//cout << "FORWARD_SMASH\n";
			} else if (horizontal > 90.0f) {
				if (!IsFacingRight()) {
					SetFacingRight(true);
				}
				attacks[Attack::FORWARD_SMASH]->Update(current_frame, IsFacingRight());
				UseAttack(Attack::FORWARD_SMASH);
				//cout << "FORWARD_SMASH\n";
			}
		}
	}

	void SmashCharacter::HandleButtonAPress() {
		if (IsInTheAir() && !has_double_jump) {
			jump_input_buffer->Start();
		} else {
			Jump();
		}
	}

	void SmashCharacter::HandleButtonARelease() {
	}

	void SmashCharacter::HandleButtonBPress() {
		weapon->Recall();
	}

	void SmashCharacter::HandleButtonBRelease() {
	}

	void SmashCharacter::HandleButtonXPress() {
		//if (IsInTheAir()) {
		//	UseAttack(Attack::NEUTRAL_AIR);
		//} else {
		//	UseAttack(Attack::JAB);
		//}
		if (weapon->Throwable()) {
			ThrowWeapon();
		}
		//cout << "JAB\n";
	}

	void SmashCharacter::HandleButtonXRelease() {
	}

	void SmashCharacter::HandleButtonYPress() {
		//if (advanced_input->DidPlayerDoQuarterCircleForward(IsFacingRight())) {
		if (!weapon->Throwable()) {
			TeleportToWeapon();
		}
	}

	void SmashCharacter::HandleButtonYRelease() {
	}

	void SmashCharacter::HandleButtonRightBumperPress() {
		//TeleportToWeapon();
	}

	void SmashCharacter::HandleButtonRightBumperRelease() {
	}

	void SmashCharacter::HandleButtonLeftBumperPress() {
	}

	void SmashCharacter::HandleButtonLeftBumperRelease() {
	}

	void SmashCharacter::HandleButtonSelectPress() {
	}

	void SmashCharacter::HandleButtonSelectRelease() {
	}

	void SmashCharacter::HandleButtonStartPress() {
	}

	void SmashCharacter::HandleButtonStartRelease() {
	}
};

#endif