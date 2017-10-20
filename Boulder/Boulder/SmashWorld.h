#pragma once

#include <iostream>
using namespace std;
#include "GameLibrary\Camera.h"
#include "GameLibrary\InputHandler.h"
#include "PlatformerLibrary\Box2D\Box2D.h"
#include "PlatformerLibrary\Box2DRigidBody.h"
#include "SmashCharacter.h"
#include "Weapon.h"
#include "Attack.h"

class MyContactListener : public b2ContactListener
{
	int contact_count = 0;

	void BeginContact(b2Contact* contact) {
		b2Fixture* fixtureA = contact->GetFixtureA();
		b2Fixture* fixtureB = contact->GetFixtureB();

		if (fixtureA->GetFilterData().categoryBits == 0x0004) {
			SmashCharacter* entityA = static_cast<SmashCharacter*>(fixtureA->GetBody()->GetUserData());
			entityA->Land();
		}
		if (fixtureB->GetFilterData().categoryBits == 0x0004) {
			SmashCharacter* entityB = static_cast<SmashCharacter*>(fixtureB->GetBody()->GetUserData());
			entityB->Land();
		}

		if (fixtureA->GetFilterData().categoryBits == 0x0020 || fixtureB->GetFilterData().categoryBits == 0x0020) {
			if (fixtureA->GetFilterData().categoryBits == 0x0020) {
				Weapon* weapon = static_cast<Weapon*>(fixtureA->GetBody()->GetUserData());
				weapon->Stick(fixtureB, weapon->GetBody()->GetAngle());
			} else if (fixtureB->GetFilterData().categoryBits == 0x0020) {
				Weapon* weapon = static_cast<Weapon*>(fixtureB->GetBody()->GetUserData());
				weapon->Stick(fixtureA, weapon->GetBody()->GetAngle());
			}
		}
	}

	void EndContact(b2Contact* contact) {
		b2Fixture* fixtureA = contact->GetFixtureA();
		b2Fixture* fixtureB = contact->GetFixtureB();

		if (fixtureA->GetFilterData().categoryBits == 0x0010) {
		}
	}

	void ConstantContact(b2Contact* contact) {
		b2Fixture* fixtureA = contact->GetFixtureA();
		b2Fixture* fixtureB = contact->GetFixtureB();

		if (fixtureA->GetFilterData().categoryBits == 0x0010) {
			SmashCharacter* entityA = static_cast<SmashCharacter*>(fixtureA->GetBody()->GetUserData());
			Attack* active_attack = entityA->GetActiveAttack();
			SmashCharacter* entityB = static_cast<SmashCharacter*>(fixtureB->GetBody()->GetUserData());

			if (active_attack->CanHitTarget(std::to_string(fixtureB->GetFilterData().categoryBits) + std::to_string(entityB->GetID()))) {
				entityB->TakeDamage(active_attack->GetDamage(), active_attack->GetKnockBack(), active_attack->GetHitStunFrames());
			}

			return;
		}

		fixtureA = contact->GetFixtureB();
		fixtureB = contact->GetFixtureA();
		
		if (fixtureA->GetFilterData().categoryBits == 0x0010) {
			SmashCharacter* entityA = static_cast<SmashCharacter*>(fixtureA->GetBody()->GetUserData());
			Attack* active_attack = entityA->GetActiveAttack();
			SmashCharacter* entityB = static_cast<SmashCharacter*>(fixtureB->GetBody()->GetUserData());

			if (active_attack->CanHitTarget(std::to_string(fixtureB->GetFilterData().categoryBits) + std::to_string(entityB->GetID()))) {
				entityB->TakeDamage(active_attack->GetDamage(), active_attack->GetKnockBack(), active_attack->GetHitStunFrames());
			}

			return;
		}
	}
};

class SmashWorld {
private:
	sf::Int64 current_frame;
	sf::RenderWindow* render_window;
	Camera* camera;
	Box2DRigidBody* test_drawable_rb;
	b2Vec2* gravity;
	b2World* world;
	b2BodyDef groundBodyDef;
	b2Body* groundBody;
	b2PolygonShape groundBox;
	b2BodyDef bodyDef;
	b2Body* body;
	b2PolygonShape dynamicBox;
	b2FixtureDef fixtureDef;
	float32 timeStep;
	int32 velocityIterations;
	int32 positionIterations;
	SmashCharacter* PlayerOne;
	InputHandler* player_one_input;
	SmashCharacter* PlayerTwo;
	InputHandler* player_two_input;
	bool exit_multiplayer;
	void ExitMultiplayer();
	MyContactListener myContactListenerInstance;
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
		WEAPON = 0x0020
		//RADAR_SENSOR = 0x0040,
	};
}; 