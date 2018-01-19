#pragma once

using namespace std;
#include "stdafx.h"
#include <iostream>
#include "Weapon.h"  
#include "SmashWorld.h"
//#include "SmashCharacter.h"

Weapon::Weapon(sf::RenderWindow *window, sf::Vector2f position, sf::Vector2f dimensions, bool subject_to_gravity, int player_idx, b2Body* owners_body/*, SmashCharacter* player*/) : Box2DRigidBody(window, subject_to_gravity) {
	player_index = player_idx;
	life_stolen = 0;

	//Player = player;

	weaponBodyDef.type = b2_dynamicBody;
	weaponBodyDef.position.Set(position.x, position.y);

	weaponBody = Singleton<SmashWorld>::Get()->GetB2World()->CreateBody(&weaponBodyDef);

	weaponShape.SetAsBox(dimensions.x / 2.0f, dimensions.y / 2.0f);
	weaponFixtureDef.isSensor = true;
	weaponFixtureDef.m_color = new b2Color(1.0f, 1.0f, 1.0f, 1.0f);
	weaponFixtureDef.shape = &weaponShape;
	weaponFixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->WEAPON;

	if (player_index == 0) {
		weaponFixtureDef.filter.maskBits = Singleton<SmashWorld>::Get()->PLATFORM | Singleton<SmashWorld>::Get()->DOOR | Singleton<SmashWorld>::Get()->ENEMY;
	} else {
		weaponFixtureDef.filter.maskBits = Singleton<SmashWorld>::Get()->PLATFORM | Singleton<SmashWorld>::Get()->DOOR | Singleton<SmashWorld>::Get()->PLAYER_CHARACTER;
	}

	weaponFixture = weaponBody->CreateFixture(&weaponFixtureDef);
	weaponBody->SetUserData(this);
	
	ownersBody = owners_body;
	gravityScale = weaponBody->GetGravityScale();

	stuck = false;
	recalling = false;
	held_by_owner = true;

	weaponBody->SetGravityScale(0.0f);
}

void Weapon::Update(sf::Int64 curr_frame, sf::Int64 delta_time) {
	Box2DRigidBody::Update(curr_frame, delta_time);

	if (held_by_owner) {
		weaponBody->SetTransform(b2Vec2(ownersBody->GetPosition().x, ownersBody->GetPosition().y), 0.0f);
	} else if (stuck) {
		weaponBody->SetTransform(b2Vec2(stuck_fixture->GetBody()->GetPosition().x - stuck_position_offset.x, stuck_fixture->GetBody()->GetPosition().y - stuck_position_offset.y), stuck_angle);
	} else if (recalling) {
		b2Vec2 recall_velocity = b2Vec2(ownersBody->GetPosition().x - weaponBody->GetPosition().x, ownersBody->GetPosition().y - weaponBody->GetPosition().y);

		if (recall_velocity.Length() < 0.25f) {
			recalling = false;
			held_by_owner = true;
			stuck_to_door = false;
			weaponBody->SetGravityScale(0.0f);
			weaponBody->SetLinearVelocity(b2Vec2(0.0f, 0.0f));

			SmashCharacter* entity = static_cast<SmashCharacter*>(ownersBody->GetUserData());
			entity->ReceiveHeal(life_stolen);

			life_stolen = 0;
		} else {
			recall_velocity.Normalize();
			float32 speed = 15.0f;
			weaponBody->SetLinearVelocity(b2Vec2(recall_velocity.x * speed, recall_velocity.y * speed));
		}
	}
}

void Weapon::Throw(b2Vec2 vector, b2Vec2 starting_position) {
	stuck = false;
	held_by_owner = false;
	weaponBody->SetGravityScale(gravityScale);
	weaponBody->SetTransform(starting_position, (float)(rand() % 360));
	weaponBody->SetAngularVelocity(20.0f);
	weaponBody->SetLinearVelocity(vector);
	weaponBody->SetAwake(true);
}

void Weapon::Stick(b2Fixture* stuck_fix, float angle) {
	stuck = true;
	stuck_fixture = stuck_fix;
	stuck_angle = angle;
	stuck_position_offset = b2Vec2(stuck_fix->GetBody()->GetPosition().x - GetBody()->GetPosition().x, stuck_fix->GetBody()->GetPosition().y - GetBody()->GetPosition().y);
	weaponBody->SetAwake(false);
}

void Weapon::Collision(b2Fixture* collider_fixture, float angle) {
	if (!held_by_owner) {
		if (collider_fixture->GetFilterData().categoryBits == Singleton<SmashWorld>::Get()->DOOR && !stuck_to_door) {
			Door* entity = static_cast<Door*>(collider_fixture->GetBody()->GetUserData());
			SmashCharacter* owner = static_cast<SmashCharacter*>(ownersBody->GetUserData());
			entity->TryToActivate(owner->GetName());
			Stick(collider_fixture, angle);
			stuck_to_door = true;
		} else if (recalling) {
			//if ((player_index == 0 && collider_fixture->GetFilterData().categoryBits == 0x0002) ||
			//	(player_index == 1 && collider_fixture->GetFilterData().categoryBits == 0x0001)) {
			//	SmashCharacter* entity = static_cast<SmashCharacter*>(collider_fixture->GetBody()->GetUserData());
			//	entity->TakeDamage(2, sf::Vector2f(-weaponBody->GetLinearVelocity().y / 2.0f, abs(weaponBody->GetLinearVelocity().x) / -2.0f), 10);
			//}
			if (collider_fixture->GetFilterData().categoryBits == 0x0002) {
				BoulderCreature* entity = static_cast<BoulderCreature*>(collider_fixture->GetBody()->GetUserData());
				life_stolen += entity->TakeDamageWithLifeSteal(2, sf::Vector2f(-weaponBody->GetLinearVelocity().y / 2.0f, abs(weaponBody->GetLinearVelocity().x) / -2.0f), 10);
			}
		} else {
			Stick(collider_fixture, angle);
		}
	}
}

void Weapon::Recall() {
	stuck = false;
	recalling = true;
	weaponBody->SetGravityScale(0.0f);
}

void Weapon::TeleportedTo() {
	stuck = false;
	recalling = false;
	held_by_owner = true;
	weaponBody->SetGravityScale(0.0f);
	weaponBody->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
}

bool Weapon::Throwable() {
	return held_by_owner;
}