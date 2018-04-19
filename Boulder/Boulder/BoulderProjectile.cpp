#pragma once

using namespace std;
#include "stdafx.h"
#include <iostream>
#include "BoulderProjectile.h"
#include "SmashWorld.h"
#include "Utilities.h"
#include <sstream>

BoulderProjectile::BoulderProjectile(sf::RenderWindow* window, b2Body* throwers_body, Json::Value projectile_json_data, SpriteAnimation* travel_anim, SpriteAnimation* on_hit_anim) {
	//"Name": "Hit 0",
	//"Box" : "11,17.5,31,33",
	//"DrawRect" : "736,516,31,33",
	//"Frame" : 0,
	//"Damage" : 50,
	//"KnockBackX" : 3.0,
	//"KnockBackY" : -3.0,
	//"ProjectileSpeedX" : 4.0,
	//"ProjectileSpeedY" : 4.0
	render_window = window;

	damage = projectile_json_data["Damage"].asInt();
	//projectile_spawn_frame;
	//projectile_spawn_relative_position;
	travel_vector = sf::Vector2f(projectile_json_data["ProjectileSpeedX"].asFloat(), projectile_json_data["ProjectileSpeedY"].asFloat());
	knock_back = sf::Vector2f(projectile_json_data["KnockBackX"].asFloat(), projectile_json_data["KnockBackY"].asFloat());

	state = STATE_INACTIVE;

	travel_animation = travel_anim;
	on_hit_animation = on_hit_anim;

	dying_status_timer = new StatusTimer(on_hit_animation->GetNumberOfFrames());

	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(0.0f, 0.0f);
	body = Singleton<SmashWorld>::Get()->GetB2World()->CreateBody(&bodyDef);
	body->SetGravityScale(0.0f);

	//float boxScale = 40.0f;
	//std::vector<float> box;
	//string strBoxValues = projectile_json_data["Box"].asString();
	//
	//if (strBoxValues != "") {
	//	std::stringstream ss_box(strBoxValues);
	//	float box_character;
	//
	//	while (ss_box >> box_character)
	//	{
	//		box.push_back(box_character);
	//
	//		if (ss_box.peek() == ',')
	//			ss_box.ignore();
	//	}
	//} else {
	//	box.push_back(1.0f);
	//	box.push_back(1.0f);
	//	box.push_back(1.0f);
	//	box.push_back(1.0f);
	//}
	//
	//float relative_x = ((box[0] + (box[2] / 2.0f)) / boxScale);
	//float relative_y = ((box[1] + (box[3] / 2.0f) - (box[1] + (box[3] / 2.0f))) / boxScale);// -(radius * 30.0f);
	//float width = (box[2]) / boxScale;
	//float height = (box[3]) / boxScale;

	std::vector<float> box = Utilities::StringBoxDataToUsableVectorBoxData(projectile_json_data["Box"].asString());

	float relative_x = box[0];
	float relative_y = box[1];
	float width = box[2];
	float height = box[3];

	shape.SetAsBox(width, height);
	fixture_def.shape = &shape;
	fixture_def.density = 1.0f;
	fixture_def.friction = 0.3f;
	fixture_def.isSensor = true;
	fixture_def.m_color = new b2Color(0.0f, 1.0f, 0.0f, 1.0f);

	fixture_def.filter.categoryBits = Singleton<SmashWorld>::Get()->PROJECTILE;
	fixture_def.filter.maskBits = Singleton<SmashWorld>::Get()->PLAYER_CHARACTER | Singleton<SmashWorld>::Get()->PLATFORM;

	fixture = body->CreateFixture(&fixture_def);

	body->SetFixedRotation(true);
	body->SetUserData(this);

	if (projectile_json_data["ArcProjectile"].asBool()) {
		isArcProjectile = true;
	}
}

void BoulderProjectile::Update() {//sf::Int64 curr_frame, sf::Int64 delta_time) {
	if (state == STATE_DYING) {
		if (!dying_status_timer->IsActive()) {
			state = STATE_INACTIVE;
		}
	}
}

void BoulderProjectile::Draw(sf::Vector2f camera_position) {
	float half_height = ((b2PolygonShape*)fixture->GetShape())->m_vertices[3].y;

	if (IsActive()) {
		if (state == STATE_TRAVELLING) {
			if (is_facing_right != travel_animation->IsFacingRight()) {
				travel_animation->Flip();
			}
			travel_animation->Draw(camera_position, sf::Vector2f((body->GetPosition().x), (body->GetPosition().y)), half_height);
		} 
	} else if (state == STATE_DYING) {
		if (is_facing_right != on_hit_animation->IsFacingRight()) {
			on_hit_animation->Flip();
		}
		on_hit_animation->Draw(camera_position, sf::Vector2f((body->GetPosition().x), (body->GetPosition().y)), half_height);
	}
}

bool BoulderProjectile::IsActive() {
	return state != STATE_INACTIVE && state != STATE_DYING;
}

void BoulderProjectile::Activate(float pos_x, float pos_y, bool facing_right) {
	state = STATE_TRAVELLING;

	body->SetTransform(b2Vec2(pos_x, pos_y), 0);
	body->SetLinearVelocity(b2Vec2(facing_right ? travel_vector.x : -travel_vector.x, travel_vector.y));

	if (isArcProjectile) {
		body->SetGravityScale(1.0f);
	}

	is_facing_right = facing_right;
}

void BoulderProjectile::Hit() {
	if (IsActive()) {
		state = STATE_DYING;

		body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));

		if (isArcProjectile) {
			body->SetGravityScale(0.0f);
		}

		dying_status_timer->Start();
	}
}

int BoulderProjectile::GetDamage() {
	return damage;
}

sf::Vector2f BoulderProjectile::GetKnockBack() {
	return knock_back;
}
int BoulderProjectile::GetHitStunFrames() {
	return (int)(knock_back.x * knock_back.y);
}