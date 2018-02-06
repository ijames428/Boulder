#pragma once

#include <iostream>
using namespace std;
#include "GameLibrary\Camera.h"
#include "GameLibrary\InputHandler.h"
#include "PlatformerLibrary\Box2D\Box2D.h"
#include "PlatformerLibrary\Box2DRigidBody.h"
#include "SmashCharacter.h"
#include "BoulderCreature.h"
#include "Weapon.h"
#include "Attack.h"
#include "Door.h"
#include "GameLibrary\Json\json.h"
#include "PlatformerLibrary\Drawable.h"
#include "DialogueLine.h"
#include "MenuController.h"
#include "Trigger.h"
#ifdef _DEBUG
#include "sfeMovie\Movie.hpp"
#else
#endif
#include "Menu.h"
#include "BossOne.h"

class MyContactListener : public b2ContactListener
{
	// PLAYER_CHARACTER = 0x0001,
	// ENEMY = 0x0002,
	// GROUND_CHECK = 0x0004,
	// PLATFORM = 0x0008,
	// HIT_BOX = 0x0010,
	// WEAPON = 0x0020,
	// DOOR = 0x0040
	int contact_count = 0;

	void BeginContact(b2Contact* contact) {
		b2Fixture* fixtureA = contact->GetFixtureA();
		b2Fixture* fixtureB = contact->GetFixtureB();

		if (fixtureA->GetFilterData().categoryBits == 0x0004) {
			if (fixtureB->GetFilterData().categoryBits == 0x0008) { // PLATFORM
				BoulderCreature* entityA = static_cast<BoulderCreature*>(fixtureA->GetBody()->GetUserData());
				entityA->Land();
				cout << "A Landed\n";
			}
		} else if (fixtureB->GetFilterData().categoryBits == 0x0004) {
			if (fixtureA->GetFilterData().categoryBits == 0x0008) { // PLATFORM
				BoulderCreature* entityB = static_cast<BoulderCreature*>(fixtureB->GetBody()->GetUserData());
				entityB->Land();
				cout << "B Landed\n";
			}
		}

		if (fixtureA->GetFilterData().categoryBits == 0x0080) {
			BoulderCreature* entityA = static_cast<BoulderCreature*>(fixtureA->GetBody()->GetUserData());
			BoulderCreature* entityB = static_cast<BoulderCreature*>(fixtureB->GetBody()->GetUserData());
			entityA->Aggro(entityB);
		} else if (fixtureB->GetFilterData().categoryBits == 0x0080) {
			BoulderCreature* entityB = static_cast<BoulderCreature*>(fixtureB->GetBody()->GetUserData());
			BoulderCreature* entityA = static_cast<BoulderCreature*>(fixtureA->GetBody()->GetUserData());
			entityB->Aggro(entityA);
		}

		if (fixtureA->GetFilterData().categoryBits == 0x0200) {
			BoulderCreature* entityA = static_cast<BoulderCreature*>(fixtureA->GetBody()->GetUserData());
			BoulderCreature* entityB = static_cast<BoulderCreature*>(fixtureB->GetBody()->GetUserData());
			entityB->SetInteractable(entityA);
		} else if (fixtureB->GetFilterData().categoryBits == 0x0200) {
			BoulderCreature* entityB = static_cast<BoulderCreature*>(fixtureB->GetBody()->GetUserData());
			BoulderCreature* entityA = static_cast<BoulderCreature*>(fixtureA->GetBody()->GetUserData());
			entityA->SetInteractable(entityB);
		}

		if (fixtureA->GetFilterData().categoryBits == 0x0400) {
			Trigger* entityA = static_cast<Trigger*>(fixtureA->GetBody()->GetUserData());
			entityA->Triggered();
		} else if (fixtureB->GetFilterData().categoryBits == 0x0400) {
			Trigger* entityB = static_cast<Trigger*>(fixtureB->GetBody()->GetUserData());
			entityB->Triggered();
		}

		//if (fixtureA->GetFilterData().categoryBits == 0x0020 || fixtureB->GetFilterData().categoryBits == 0x0020) {
		//	if (fixtureA->GetFilterData().categoryBits == 0x0020) {
		//		Weapon* weapon = static_cast<Weapon*>(fixtureA->GetBody()->GetUserData());
		//		weapon->Collision(fixtureB, weapon->GetBody()->GetAngle());
		//	} else if (fixtureB->GetFilterData().categoryBits == 0x0020) {
		//		Weapon* weapon = static_cast<Weapon*>(fixtureB->GetBody()->GetUserData());
		//		weapon->Collision(fixtureA, weapon->GetBody()->GetAngle());
		//	}
		//}
	}

	void EndContact(b2Contact* contact) {
		b2Fixture* fixtureA = contact->GetFixtureA();
		b2Fixture* fixtureB = contact->GetFixtureB();

		if (fixtureA->GetFilterData().categoryBits == 0x0010) {
		}

		if (fixtureA->GetFilterData().categoryBits == 0x0100) {
			BoulderCreature* entityA = static_cast<BoulderCreature*>(fixtureA->GetBody()->GetUserData());
			entityA->Deaggro();
		}
		if (fixtureB->GetFilterData().categoryBits == 0x0100) {
			BoulderCreature* entityB = static_cast<BoulderCreature*>(fixtureB->GetBody()->GetUserData());
			entityB->Deaggro();
		}

		if (fixtureA->GetFilterData().categoryBits == 0x0200) {
			BoulderCreature* entityA = static_cast<BoulderCreature*>(fixtureA->GetBody()->GetUserData());
			BoulderCreature* entityB = static_cast<BoulderCreature*>(fixtureB->GetBody()->GetUserData());
			entityB->SetInteractable(nullptr);
		}
		if (fixtureB->GetFilterData().categoryBits == 0x0200) {
			BoulderCreature* entityB = static_cast<BoulderCreature*>(fixtureB->GetBody()->GetUserData());
			BoulderCreature* entityA = static_cast<BoulderCreature*>(fixtureA->GetBody()->GetUserData());
			entityA->SetInteractable(nullptr);
		}
	}

	void ConstantContact(b2Contact* contact) {
		b2Fixture* fixtureA = contact->GetFixtureA();
		b2Fixture* fixtureB = contact->GetFixtureB();

		if (fixtureA->GetFilterData().categoryBits == 0x0010) {
			BoulderCreature* entityA = static_cast<BoulderCreature*>(fixtureA->GetBody()->GetUserData());
			Attack* active_attack = entityA->GetActiveAttack();
			BoulderCreature* entityB = static_cast<BoulderCreature*>(fixtureB->GetBody()->GetUserData());

			if (active_attack->CanHitTarget(std::to_string(fixtureB->GetFilterData().categoryBits) + std::to_string(entityB->GetID()))) {
				entityB->TakeDamage(active_attack->GetDamage(), active_attack->GetKnockBack(), active_attack->GetHitStunFrames(), active_attack->IsPopUpMove());
			}

			return;
		}

		fixtureA = contact->GetFixtureB();
		fixtureB = contact->GetFixtureA();
		
		if (fixtureA->GetFilterData().categoryBits == 0x0010) {
			BoulderCreature* entityA = static_cast<BoulderCreature*>(fixtureA->GetBody()->GetUserData());
			Attack* active_attack = entityA->GetActiveAttack();
			BoulderCreature* entityB = static_cast<BoulderCreature*>(fixtureB->GetBody()->GetUserData());

			if (active_attack->CanHitTarget(std::to_string(fixtureB->GetFilterData().categoryBits) + std::to_string(entityB->GetID()))) {
				entityB->TakeDamage(active_attack->GetDamage(), active_attack->GetKnockBack(), active_attack->GetHitStunFrames(), active_attack->IsPopUpMove());
			}

			return;
		}

		if (fixtureA->GetFilterData().categoryBits == 0x0020 || fixtureB->GetFilterData().categoryBits == 0x0020) {
			if (fixtureA->GetFilterData().categoryBits == 0x0020) {
				Weapon* weapon = static_cast<Weapon*>(fixtureA->GetBody()->GetUserData());
				weapon->Collision(fixtureB, weapon->GetBody()->GetAngle());
			} else if (fixtureB->GetFilterData().categoryBits == 0x0020) {
				Weapon* weapon = static_cast<Weapon*>(fixtureB->GetBody()->GetUserData());
				weapon->Collision(fixtureA, weapon->GetBody()->GetAngle());
			}
		}

		if (fixtureA->GetFilterData().categoryBits == 0x0800) { // PROJECTILE
			BoulderProjectile* entityA = static_cast<BoulderProjectile*>(fixtureA->GetBody()->GetUserData());

			if (entityA->IsActive()) {
				entityA->Hit();

				if (fixtureB->GetFilterData().categoryBits == 0x0001) { // PLAYER_CHARACTER
					BoulderCreature* entityB = static_cast<BoulderCreature*>(fixtureB->GetBody()->GetUserData());
					entityB->TakeDamage(entityA->GetDamage(), entityA->GetKnockBack(), entityA->GetHitStunFrames(), false);
				}
			}
		}
		else if (fixtureB->GetFilterData().categoryBits == 0x0800) { // PROJECTILE
			BoulderProjectile* entityB = static_cast<BoulderProjectile*>(fixtureB->GetBody()->GetUserData());

			if (entityB->IsActive()) {
				entityB->Hit();

				if (fixtureA->GetFilterData().categoryBits == 0x0001) { // PLAYER_CHARACTER
					BoulderCreature* entityA = static_cast<BoulderCreature*>(fixtureA->GetBody()->GetUserData());
					entityA->TakeDamage(entityB->GetDamage(), entityB->GetKnockBack(), entityB->GetHitStunFrames(), false);
				}
			}
		}
	}
};

class SmashWorld {
private:
	struct DeserializedRectangle {
		string name;
		float x;
		float y;
		float width;
		float height;
	};

	string STAGE_OF_GAME_INTRO = "Intro";
	string STAGE_OF_GAME_OPENED_UP = "Opened Up";
	string STAGE_OF_GAME_AFTER_FIRST_BOSS = "After First Boss";

	string StageOfTheGame;
	sf::Int64 current_frame;
	sf::RenderWindow* render_window;
	Camera* camera;
	Drawable* test_drawable;
	b2Vec2* gravity;
	b2World* world;
	float32 timeStep;
	int32 velocityIterations;
	int32 positionIterations;
	SmashCharacter* PlayerOne;
	InputHandler* player_character_input;
	InputHandler* player_menu_input;
	bool exit_to_main_menu;
	MyContactListener myContactListenerInstance;
	void ParseWorld(string file_path);
	void ParsePlayerBestiary(string file_path);
	void ParseBestiaries();
	void ParseBestiary(string file_path);
	void ParseDialogue(string file_path);
	void BuildWorld();
	Json::Value jsonWorldData;
	Json::Value jsonPlayerData;
	std::vector<Json::Value> jsonBestiariesData;
	Json::Value jsonDialogueData;
	std::vector<Box2DRigidBody*> box2dRigidBodies;
	std::vector<Door*> doors;
	std::vector<Trigger*> triggers;
	std::vector<BoulderCreature*> enemies;
	BossOne* boss_one;
	bool boss_one_fight_started;
	sf::Font ringbearer_font;
	sf::Text dialogue_text;
	string unit_type_player_is_talking_to;
	DialogueLine* RootDialogueLine;
	DialogueLine* CurrentDialogueLine;
#ifdef _DEBUG
	sfe::Movie intro_cutscene;
#else
#endif
	bool past_setup = false;
	sf::Vector2f parallax_background_viewport_position;
	sf::Texture parallax_background_texture;
	sf::Sprite parallax_background_sprite;

	Menu* DeadMenu;
	Menu* PauseMenu;
	bool can_take_another_left_stick_input_from_menu_controller = true;

	sf::Shader shader;
	//sf::Shader lighting_shader;
public:
	SmashWorld();
	void Init(sf::RenderWindow* window, Camera* cam);
	void Update(sf::Int64 curr_frame, sf::Int64 frame_delta);
	bool ShouldExitToMainMenu();
	Camera* GetCamera() { return camera; };
	b2World* GetB2World() { return world; };
	sf::Int64 GetCurrentFrame() { return current_frame; };
	void SetDialogueText(string new_text);
	void StartDialogue(string unit_type);
	void ProgressDialogueText();
	string GetCurrentPointInGame();
	void UpdateVideo();
	void Setup();
	void ExportSaveData();
	void ImportSaveData();
	void ExitGame();
	void ExitToMainMenu(); 
	void CloseCurrentMenu();
	void PlayerDied();
	void ExecuteAction(string action_call);
	void HandleLeftStickInput(float horizontal, float vertical);
	void HandleRightStickInput(float horizontal, float vertical);
	void HandleButtonBPress();
	void HandleButtonBRelease();
	void HandleButtonXPress();
	void HandleButtonXRelease();
	void HandleButtonAPress();
	void HandleButtonARelease();
	void HandleButtonStartPress();
	void HandleButtonStartRelease();
	void HandleButtonSelectPress();
	void HandleButtonSelectRelease();
	void ScreenShake(float magnitude);

	bool Contains(string string_being_searched, string string_being_searched_for) {
		std::size_t found = string_being_searched.find(string_being_searched_for);
		return found != std::string::npos;
	}

	enum EntityCategory {
		PLAYER_CHARACTER = 0x0001,
		ENEMY = 0x0002,
		GROUND_CHECK = 0x0004,
		PLATFORM = 0x0008,
		HIT_BOX = 0x0010,
		WEAPON = 0x0020,
		DOOR = 0x0040,
		AGGRO_CIRCLE = 0x0080,
		DEAGGRO_CIRCLE = 0x0100,
		INTERACTION_CIRCLE = 0x0200,
		TRIGGER = 0x0400,
		PROJECTILE = 0x0800
	};
}; 