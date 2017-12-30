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

class MyContactListener : public b2ContactListener
{
	// PLAYER_ONE = 0x0001,
	// PLAYER_TWO = 0x0002,
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
			BoulderCreature* entityA = static_cast<BoulderCreature*>(fixtureA->GetBody()->GetUserData());
			entityA->Land();
		}
		if (fixtureB->GetFilterData().categoryBits == 0x0004) {
			BoulderCreature* entityB = static_cast<BoulderCreature*>(fixtureB->GetBody()->GetUserData());
			entityB->Land();
		}

		if (fixtureA->GetFilterData().categoryBits == 0x0080) {
			BoulderCreature* entityA = static_cast<BoulderCreature*>(fixtureA->GetBody()->GetUserData());
			BoulderCreature* entityB = static_cast<BoulderCreature*>(fixtureB->GetBody()->GetUserData());
			entityA->Aggro(entityB);
		}
		if (fixtureB->GetFilterData().categoryBits == 0x0080) {
			BoulderCreature* entityB = static_cast<BoulderCreature*>(fixtureB->GetBody()->GetUserData());
			BoulderCreature* entityA = static_cast<BoulderCreature*>(fixtureA->GetBody()->GetUserData());
			entityB->Aggro(entityA);
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
	}

	void ConstantContact(b2Contact* contact) {
		b2Fixture* fixtureA = contact->GetFixtureA();
		b2Fixture* fixtureB = contact->GetFixtureB();

		if (fixtureA->GetFilterData().categoryBits == 0x0010) {
			BoulderCreature* entityA = static_cast<BoulderCreature*>(fixtureA->GetBody()->GetUserData());
			Attack* active_attack = entityA->GetActiveAttack();
			BoulderCreature* entityB = static_cast<BoulderCreature*>(fixtureB->GetBody()->GetUserData());

			if (active_attack->CanHitTarget(std::to_string(fixtureB->GetFilterData().categoryBits) + std::to_string(entityB->GetID()))) {
				entityB->TakeDamage(active_attack->GetDamage(), active_attack->GetKnockBack(), active_attack->GetHitStunFrames());
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
				entityB->TakeDamage(active_attack->GetDamage(), active_attack->GetKnockBack(), active_attack->GetHitStunFrames());
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
	InputHandler* player_one_input;
	bool exit_multiplayer;
	void ExitMultiplayer();
	MyContactListener myContactListenerInstance;
	void ParseWorld(string file_path);
	void ParseBestiaries();
	void ParseBestiary(string file_path);
	void BuildWorld();
	Json::Value jsonWorldData;
	string rawWorldData;
	std::vector<Json::Value> jsonBestiariesData;
	//std::vector<string> rawBestiariesData;
	std::vector<Box2DRigidBody*> box2dRigidBodies;
	std::vector<Door*> doors;
	std::vector<BoulderCreature*> enemies;
public:
	SmashWorld();
	void Init(sf::RenderWindow* window, Camera* cam);
	void Update(sf::Int64 curr_frame, sf::Int64 frame_delta);
	bool ShouldExitMultiplayer();
	Camera* GetCamera() { return camera; };
	b2World* GetB2World() { return world; };
	sf::Int64 GetCurrentFrame() { return current_frame; };
	enum EntityCategory {
		PLAYER_ONE = 0x0001,
		PLAYER_TWO = 0x0002,
		GROUND_CHECK = 0x0004,
		PLATFORM = 0x0008,
		HIT_BOX = 0x0010,
		WEAPON = 0x0020,
		DOOR = 0x0040,
		AGGRO_CIRCLE = 0x0080,
		DEAGGRO_CIRCLE = 0x0100
	};
}; 