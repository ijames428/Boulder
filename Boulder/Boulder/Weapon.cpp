#pragma once

using namespace std;
#include "stdafx.h"
#include <iostream>
#include "Weapon.h"  
#include "SmashWorld.h"

Weapon::Weapon(sf::RenderWindow *window, sf::Vector2f position, sf::Vector2f dimensions, bool subject_to_gravity, int player_idx, b2Body* owners_body) : Box2DRigidBody(window, position, dimensions, subject_to_gravity) {
	player_index = player_idx;

	weaponBodyDef.type = b2_dynamicBody;
	weaponBodyDef.position.Set(position.x, position.y);

	weaponBody = Singleton<SmashWorld>::Get()->GetB2World()->CreateBody(&weaponBodyDef);

	weaponShape.SetAsBox(dimensions.x / 2.0f, dimensions.y / 2.0f);
	weaponFixtureDef.isSensor = true;
	weaponFixtureDef.m_color = new b2Color(1.0f, 1.0f, 1.0f, 1.0f);
	weaponFixtureDef.shape = &weaponShape;
	weaponFixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->WEAPON;

	if (player_index == 0) {
		weaponFixtureDef.filter.maskBits = Singleton<SmashWorld>::Get()->PLATFORM | Singleton<SmashWorld>::Get()->PLAYER_TWO;
	} else {
		weaponFixtureDef.filter.maskBits = Singleton<SmashWorld>::Get()->PLATFORM | Singleton<SmashWorld>::Get()->PLAYER_ONE;
	}

	weaponFixture = weaponBody->CreateFixture(&weaponFixtureDef);
	weaponBody->SetUserData(this);
	
	ownersBody = owners_body;
	gravityScale = weaponBody->GetGravityScale();

	stuck = false;
	recalling = false;
}

void Weapon::Update(sf::Int64 curr_frame, sf::Int64 delta_time) {
	Box2DRigidBody::Update(curr_frame, delta_time);

	if (stuck) {
		weaponBody->SetTransform(b2Vec2(stuck_fixture->GetBody()->GetPosition().x - stuck_position_offset.x, stuck_fixture->GetBody()->GetPosition().y - stuck_position_offset.y), stuck_angle);
	} else if (recalling) {
		b2Vec2 recall_velocity = b2Vec2(ownersBody->GetPosition().x - weaponBody->GetPosition().x, ownersBody->GetPosition().y - weaponBody->GetPosition().y);

		if (recall_velocity.Length() < 0.25f) {
			recalling = false;
			weaponBody->SetGravityScale(gravityScale);
			weaponBody->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
		} else {
			recall_velocity.Normalize();
			float32 speed = 15.0f;
			weaponBody->SetLinearVelocity(b2Vec2(recall_velocity.x * speed, recall_velocity.y * speed));
		}
		
		cout << "" << weaponBody->GetLinearVelocity().x << ", " << weaponBody->GetLinearVelocity().y << "\n";
	}
}

void Weapon::Throw(b2Vec2 vector, b2Vec2 starting_position) {
	stuck = false;
	weaponBody->SetGravityScale(gravityScale);
	weaponBody->SetTransform(starting_position, rand() % 360);
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

void Weapon::Recall() {
	stuck = false;
	recalling = true;
	weaponBody->SetGravityScale(0.0f);
}

void Weapon::TeleportedTo() {
	stuck = false;
	recalling = false;
	weaponBody->SetGravityScale(0.0f);
	weaponBody->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
}