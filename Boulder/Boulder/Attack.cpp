#pragma once

using namespace std;
#include "stdafx.h"
#include <iostream>
#include "Attack.h"
#include "GameLibrary\Singleton.h"
#include "SmashWorld.h"
#include <vector>
#include <string>
#include <sstream>

#define DEGTORAD 0.0174532925199432957f

Attack::Attack(b2Body* body, int index, int move_type) {
	player_body = body;
	player_index = index;

	attack_frames = GetAttackFrames(move_type);

	attack_timer = new StatusTimer((int)attack_frames.size());

	enemies_hit = std::vector<string>();
}

Attack::Attack(b2Body* body, int index, int move_type, Json::Value jsonData) {
	player_body = body;
	player_index = index;
	float boxScale = 160.0f;

	if (!jsonData["HitBoxPerFrame"].empty())
	{
		for (int i = 0; i < jsonData["NumberOfFrames"].asInt(); i++)
		{
			attack_frames.push_back(std::vector<AttackFrame*>());

			if (!jsonData["HitBoxPerFrame"][i].empty())
			{
				for (int box = 0; box < (int)jsonData["HitBoxPerFrame"][i].size(); box++)
				{
					string name = jsonData["HitBoxPerFrame"][i][box]["Name"].asString();
					int damage = jsonData["HitBoxPerFrame"][i][box]["Damage"].asInt();
					float knockback_x = jsonData["HitBoxPerFrame"][i][box]["KnockBackX"].asFloat();
					float knockback_y = jsonData["HitBoxPerFrame"][i][box]["KnockBackY"].asFloat();
					int frames_of_hit_stun = (int)(knockback_x * knockback_y); // TODO: Have hit stun frames come from frame data once it's in there.

					frames_of_hit_stun = frames_of_hit_stun > 0 ? frames_of_hit_stun : -frames_of_hit_stun;

					std::vector<float> vect;
					string strBoxValues = jsonData["HitBoxPerFrame"][i][box]["Box"].asString();
					std::stringstream ss(strBoxValues);
					float character;

					while (ss >> character)
					{
						vect.push_back(character);

						if (ss.peek() == ',')
							ss.ignore();
					}

					float radius = player_body->GetFixtureList()[0].GetShape()->m_radius;
					float hit_box_relative_x = ((vect[0] + (vect[2] / 2.0f)) / boxScale);
					float hit_box_relative_y = ((vect[1] + (vect[3] / 2.0f)) / boxScale) - (radius * 30.0f);
					float hit_box_width = (vect[2]) / boxScale;
					float hit_box_height = (vect[3]) / boxScale;

					attack_frames[i].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), damage, frames_of_hit_stun,
															   sf::Vector2f(knockback_x, knockback_y), sf::Vector2f(hit_box_width, hit_box_height), sf::Vector2f(hit_box_relative_x, hit_box_relative_y), 
															   true));
				}
			}
			else
			{
				attack_frames[i].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false, true));
			}
		}
	}

	attack_timer = new StatusTimer((int)attack_frames.size());

	enemies_hit = std::vector<string>();
}

void Attack::Update(sf::Uint64 curr_frame, bool facing_right) {
	current_frame = curr_frame;
	current_frame_in_attack = current_frame - starting_attack_frame;

	if (IsAttacking()) {
		for (int i = 0; i < (int)attack_frames.size(); i++) {
			if (i == current_frame_in_attack) {
				for (int box = 0; box < (int)attack_frames[i].size(); box++) {
					attack_frames[i][box]->Activate();

					if (attack_frames[i][box]->IsClearingFrame()) {
						enemies_hit.clear();
					}
				}
			} else {
				for (int box = 0; box < (int)attack_frames[i].size(); box++) {
					attack_frames[i][box]->Deactivate();
				}
			}
		}
	} else {
		for (int i = 0; i < (int)attack_frames.size(); i++) {
			for (int box = 0; box < (int)attack_frames[i].size(); box++) {
				attack_frames[i][box]->Update(facing_right);
			}
		}
	}
}

void Attack::InitiateAttack() {
	current_frame_in_attack = 0;
	starting_attack_frame = current_frame;
	attack_timer->Start();
}

void Attack::StopAttack() {
	attack_timer->Stop();
	for (int i = 0; i < (int)attack_frames.size(); i++) {
		for (int box = 0; box < (int)attack_frames[i].size(); box++) {
			attack_frames[i][box]->Deactivate();
		}
	}
	enemies_hit.clear();
}

int Attack::GetDamage() {
	if (current_frame_in_attack >= attack_frames.size()) {
		return 0;
	}

	// TODO: Separate out the boxes so we can get the damage of an individual one.
	return attack_frames[(int)current_frame_in_attack][0]->GetDamage();
}

bool Attack::IsAttacking() {
	return attack_timer->IsActive();
}

sf::Vector2f Attack::GetKnockBack() {
	if (current_frame_in_attack >= attack_frames.size()) {
		return sf::Vector2f(0.0f, 0.0f);
	}

	// TODO: Separate out the boxes so we can get the knockback of an individual one.
	return attack_frames[(int)current_frame_in_attack][0]->GetKnockBack();
}

int Attack::GetHitStunFrames() {
	if (current_frame_in_attack >= attack_frames.size()) {
		return 0;
	}

	// TODO: Separate out the boxes so we can get the hit stun frames of an individual one.
	return attack_frames[(int)current_frame_in_attack][0]->GetHitStunFrames();
}

bool Attack::CanHitTarget(string identifier) {
	bool found = false;

	for (int i = 0; i < (int)enemies_hit.size(); i++) {
		if (enemies_hit[i] == identifier) {
			found = true;
		}
	}

	if (found) {
		return false;
	} else {
		enemies_hit.push_back(identifier);
		return true;
	}
}


AttackFrame::AttackFrame(b2Body* body, int player_index, int frame_in_attack, int dmg, sf::Int64 stun_frames, sf::Vector2f kfr, sf::Vector2f hit_box_dimensions, sf::Vector2f hit_box_relative_central_position, bool has_hb, bool clear_hit_enemies) {
	has_hit_box = has_hb;
	frame_for_clearing_enemies_hit = clear_hit_enemies;

	if (has_hit_box) {
		hitbox = new HitBox(body, player_index, frame_in_attack, hit_box_dimensions, hit_box_relative_central_position);
	}

	damage = dmg;
	knockback_facing_right = kfr;
	knockback_facing_left = sf::Vector2f(-kfr.x, kfr.y);
	number_of_hit_stun_frames = (int)stun_frames;
}

void AttackFrame::Update(bool facing_right) {
	if (HasHitBox()) {
		hitbox->Update(facing_right);
	}
}

void AttackFrame::Activate() {
	if (HasHitBox()) {
		hitbox->Activate();
	}
}

void AttackFrame::Deactivate() {
	if (HasHitBox()) {
		hitbox->Deactivate();
	}
}

bool AttackFrame::IsClearingFrame() {
	return frame_for_clearing_enemies_hit;
}

sf::Vector2f AttackFrame::GetKnockBack() {
	if (HasHitBox()) {
		if (hitbox->IsFacingRight()) {
			return knockback_facing_right;
		} else {
			return knockback_facing_left;
		}
	}

	return sf::Vector2f(0.0f, 0.0f);
}

int AttackFrame::GetDamage() {
	return damage;
}

int AttackFrame::GetHitStunFrames() {
	return number_of_hit_stun_frames;
}

bool AttackFrame::HasHitBox() {
	return has_hit_box;
}


HitBox::HitBox(b2Body* body, int player_index, int frame_in_attack, sf::Vector2f dimensions, sf::Vector2f relative_central_position) {
	facing_right = true;

	polygonShapeLeft.SetAsBox(dimensions.x / 2.0f, dimensions.y / 2.0f, b2Vec2(-relative_central_position.x, relative_central_position.y), 0.0f);
	myFixtureDefLeft.shape = &polygonShapeLeft;
	myFixtureDefLeft.isSensor = true;
	myFixtureDefLeft.m_color = new b2Color(1.0f, 0.0f, 1.0f, 1.0f);

	if (player_index == 0) {
		myFixtureDefLeft.filter.categoryBits = Singleton<SmashWorld>::Get()->HIT_BOX;
		myFixtureDefLeft.filter.maskBits = Singleton<SmashWorld>::Get()->PLAYER_TWO;
	} else {
		myFixtureDefLeft.filter.categoryBits = Singleton<SmashWorld>::Get()->HIT_BOX;
		myFixtureDefLeft.filter.maskBits = Singleton<SmashWorld>::Get()->PLAYER_ONE;
	}

	myFixtureLeft = body->CreateFixture(&myFixtureDefLeft);
	myFixtureLeft->SetActive(false);


	polygonShapeRight.SetAsBox(dimensions.x / 2.0f, dimensions.y / 2.0f, b2Vec2(relative_central_position.x, relative_central_position.y), 0.0f);
	myFixtureDefRight.shape = &polygonShapeRight;
	myFixtureDefRight.isSensor = true;
	myFixtureDefRight.m_color = new b2Color(1.0f, 0.0f, 1.0f, 1.0f);

	if (player_index == 0) {
		myFixtureDefRight.filter.categoryBits = Singleton<SmashWorld>::Get()->HIT_BOX;
		myFixtureDefRight.filter.maskBits = Singleton<SmashWorld>::Get()->PLAYER_TWO;
	} else {
		myFixtureDefRight.filter.categoryBits = Singleton<SmashWorld>::Get()->HIT_BOX;
		myFixtureDefRight.filter.maskBits = Singleton<SmashWorld>::Get()->PLAYER_ONE;
	}

	myFixtureRight = body->CreateFixture(&myFixtureDefRight);
	myFixtureRight->SetActive(false);

	//float radius = 1;
	//b2Vec2 verticesLeft[60];
	//verticesLeft[0].Set(0, 0);
	////counterclockwise
	//for (int i = 0; i < frame_in_attack - 1; i++) {
	//	float angle = (i + 9) / 6.0f * -90 * DEGTORAD;
	//	verticesLeft[i + 1].Set(radius * cosf(angle), radius * sinf(angle));
	//}
	//polygonShape.Set(verticesLeft, frame_in_attack);
	//myFixtureDefLeft.shape = &polygonShape;
	//myFixtureDefLeft.isSensor = true;
	//
	//if (player_index == 0) {
	//	myFixtureDefLeft.filter.categoryBits = Singleton<SmashWorld>::Get()->HIT_BOX;
	//	myFixtureDefLeft.filter.maskBits = Singleton<SmashWorld>::Get()->PLAYER_TWO;
	//} else {
	//	myFixtureDefLeft.filter.categoryBits = Singleton<SmashWorld>::Get()->HIT_BOX;
	//	myFixtureDefLeft.filter.maskBits = Singleton<SmashWorld>::Get()->PLAYER_ONE;
	//}
	//
	//myFixtureDefLeft.m_color = new b2Color(1.0f, 0.0f, 1.0f, 1.0f);
	//myFixtureLeft = body->CreateFixture(&myFixtureDefLeft);
	//myFixtureLeft->SetActive(false);
	//
	//
	//b2Vec2 verticesRight[60];
	//verticesRight[0].Set(0, 0);
	////clockwise
	//for (int i = 0; i < frame_in_attack - 1; i++) {
	//	float angle = (i - 3) / 6.0f * 90 * DEGTORAD;
	//	verticesRight[i + 1].Set(radius * cosf(angle), radius * sinf(angle));
	//}
	//polygonShape.Set(verticesRight, frame_in_attack);
	//myFixtureDefRight.shape = &polygonShape;
	//myFixtureDefRight.isSensor = true;
	//
	//if (player_index == 0) {
	//	myFixtureDefRight.filter.categoryBits = Singleton<SmashWorld>::Get()->HIT_BOX;
	//	myFixtureDefRight.filter.maskBits = Singleton<SmashWorld>::Get()->PLAYER_TWO;
	//}
	//else {
	//	myFixtureDefRight.filter.categoryBits = Singleton<SmashWorld>::Get()->HIT_BOX;
	//	myFixtureDefRight.filter.maskBits = Singleton<SmashWorld>::Get()->PLAYER_ONE;
	//}
	//
	//myFixtureDefRight.m_color = new b2Color(1.0f, 0.0f, 1.0f, 1.0f);
	//myFixtureRight = body->CreateFixture(&myFixtureDefRight);
	//myFixtureRight->SetActive(false);
}

void HitBox::Update(bool new_facing_right) {
	facing_right = new_facing_right;
}

bool HitBox::IsFacingRight() {
	return facing_right;
}

void HitBox::Activate() {
	if (facing_right) {
		myFixtureRight->SetActive(true);
	} else {
		myFixtureLeft->SetActive(true);
	}
}

void HitBox::Deactivate() {
	if (facing_right) {
		myFixtureRight->SetActive(false);
	} else {
		myFixtureLeft->SetActive(false);
	}
}



std::vector<std::vector<AttackFrame*>> Attack::GetAttackFrames(int move) {
	std::vector<std::vector<AttackFrame*>> attack_frames = std::vector<std::vector<AttackFrame*>>();

	switch (move) {
	case JAB:
		attack_frames = MakeMoveJab();
		break;
	case UP_SMASH:
		attack_frames = MakeMoveUpSmash();
		break;
	case DOWN_SMASH:
		attack_frames = MakeMoveDownSmash();
		break;
	case FORWARD_SMASH:
		attack_frames = MakeMoveForwardSmash();
		break;
	case UP_AIR:
		attack_frames = MakeMoveUpAir();
		break;
	case DOWN_AIR:
		attack_frames = MakeMoveDownAir();
		break;
	case FORWARD_AIR:
		attack_frames = MakeMoveForwardAir();
		break;
	case BACK_AIR:
		attack_frames = MakeMoveBackAir();
		break;
	case NEUTRAL_AIR:
		attack_frames = MakeMoveNeutralAir();
		break;
	case DASH_ATTACK:
		attack_frames = MakeMoveJab();
		break;
	//case DASH_PUNCH:
	//	attack_frames = MakeMoveJab();
	//	break;
	//case THROW_WEAPON:
	//	attack_frames = MakeMoveJab();
	//	break;
	//case TELEPORT_TO_WEAPON:
	//	attack_frames = MakeMoveJab();
	//	break;
	//case URIENS_SUPER:
	//	attack_frames = MakeMoveJab();
	//	break;
	default:
		break;
	}

	return attack_frames;
}

std::vector<std::vector<AttackFrame*>> Attack::MakeMoveJab() {
	std::vector<std::vector<AttackFrame*>> attack_frames = std::vector<std::vector<AttackFrame*>>();

	// startup frames
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false, true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));

	// active frames
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 10, 3, sf::Vector2f(2.0f, -5.0f), sf::Vector2f(0.3f, 0.15f), sf::Vector2f(0.3f, -0.25f), true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 20, 6, sf::Vector2f(2.0f, -5.0f), sf::Vector2f(0.3f, 0.15f), sf::Vector2f(0.3f, -0.25f), true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 10, 3, sf::Vector2f(2.0f, -5.0f), sf::Vector2f(0.3f, 0.15f), sf::Vector2f(0.3f, -0.25f), true));

	// recovery frames
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false, true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));

	return attack_frames;
}

std::vector<std::vector<AttackFrame*>> Attack::MakeMoveUpAir() {
	std::vector<std::vector<AttackFrame*>> attack_frames = std::vector<std::vector<AttackFrame*>>();

	// startup frames
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false, true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));

	// active frames
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 20, 6, sf::Vector2f(1.0f, -10.0f), sf::Vector2f(0.1f, 0.4f), sf::Vector2f(0.3f, -0.5f), true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 20, 6, sf::Vector2f(1.0f, -10.0f), sf::Vector2f(0.1f, 0.4f), sf::Vector2f(0.3f, -0.5f), true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 20, 6, sf::Vector2f(1.0f, -10.0f), sf::Vector2f(0.1f, 0.4f), sf::Vector2f(0.3f, -0.5f), true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 20, 6, sf::Vector2f(1.0f, -10.0f), sf::Vector2f(0.1f, 0.4f), sf::Vector2f(0.3f, -0.5f), true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 20, 6, sf::Vector2f(1.0f, -10.0f), sf::Vector2f(0.1f, 0.4f), sf::Vector2f(0.3f, -0.5f), true));

	// recovery frames
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false, true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));

	return attack_frames;
}

std::vector<std::vector<AttackFrame*>> Attack::MakeMoveUpSmash() {
	std::vector<std::vector<AttackFrame*>> attack_frames = std::vector<std::vector<AttackFrame*>>();

	// startup frames
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false, true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));

	// active frames
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 10, 3, sf::Vector2f(0.0f, -2.0f), sf::Vector2f(0.1f, 0.2f), sf::Vector2f(0.0f, -0.5f), true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 20, 6, sf::Vector2f(0.0f, -2.0f), sf::Vector2f(0.1f, 0.2f), sf::Vector2f(0.0f, -0.5f), true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 10, 3, sf::Vector2f(0.0f, -2.0f), sf::Vector2f(0.1f, 0.2f), sf::Vector2f(0.0f, -0.5f), true));

	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false, true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));

	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 10, 3, sf::Vector2f(0.0f, -8.0f), sf::Vector2f(0.1f, 0.3f), sf::Vector2f(0.0f, -0.5f), true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 20, 6, sf::Vector2f(0.0f, -8.0f), sf::Vector2f(0.1f, 0.3f), sf::Vector2f(0.0f, -0.5f), true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 10, 3, sf::Vector2f(0.0f, -8.0f), sf::Vector2f(0.1f, 0.3f), sf::Vector2f(0.0f, -0.5f), true));

	// recovery frames
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false, true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));

	return attack_frames;
}

std::vector<std::vector<AttackFrame*>> Attack::MakeMoveNeutralAir() {
	std::vector<std::vector<AttackFrame*>> attack_frames = std::vector<std::vector<AttackFrame*>>();

	// startup frames
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false, true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));

	// active frames
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 10, 10, sf::Vector2f(0.75f, -1.0f), sf::Vector2f(0.1f, 0.2f), sf::Vector2f(0.2f, -0.5f), true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 10, 10, sf::Vector2f(0.75f, -1.0f), sf::Vector2f(0.1f, 0.2f), sf::Vector2f(0.2f, -0.5f), true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 10, 10, sf::Vector2f(0.75f, -1.0f), sf::Vector2f(0.1f, 0.2f), sf::Vector2f(0.2f, -0.5f), true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 10, 10, sf::Vector2f(0.75f, -1.0f), sf::Vector2f(0.1f, 0.2f), sf::Vector2f(0.2f, -0.5f), true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 10, 10, sf::Vector2f(0.75f, -1.0f), sf::Vector2f(0.1f, 0.2f), sf::Vector2f(0.2f, -0.5f), true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 10, 10, sf::Vector2f(0.75f, -1.0f), sf::Vector2f(0.1f, 0.2f), sf::Vector2f(0.2f, -0.5f), true));

	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false, true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));

	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 20, 3, sf::Vector2f(2.0f, 4.0f), sf::Vector2f(0.1f, 0.3f), sf::Vector2f(0.2f, -0.5f), true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 20, 3, sf::Vector2f(2.0f, 4.0f), sf::Vector2f(0.1f, 0.3f), sf::Vector2f(0.2f, -0.5f), true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 20, 3, sf::Vector2f(2.0f, 4.0f), sf::Vector2f(0.1f, 0.3f), sf::Vector2f(0.2f, -0.5f), true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 20, 3, sf::Vector2f(2.0f, 4.0f), sf::Vector2f(0.1f, 0.3f), sf::Vector2f(0.2f, -0.5f), true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 20, 3, sf::Vector2f(2.0f, 4.0f), sf::Vector2f(0.1f, 0.3f), sf::Vector2f(0.2f, -0.5f), true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 20, 3, sf::Vector2f(2.0f, 4.0f), sf::Vector2f(0.1f, 0.3f), sf::Vector2f(0.2f, -0.5f), true));

	// recovery frames
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false, true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));

	return attack_frames;
}

std::vector<std::vector<AttackFrame*>> Attack::MakeMoveDownSmash() {
	std::vector<std::vector<AttackFrame*>> attack_frames = std::vector<std::vector<AttackFrame*>>();

	// startup frames
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false, true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));

	// active frames
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 80, 8, sf::Vector2f(2.0f, -8.0f), sf::Vector2f(0.3f, 1.0f), sf::Vector2f(0.1f, 0.0f), true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 80, 8, sf::Vector2f(2.0f, -8.0f), sf::Vector2f(0.3f, 1.0f), sf::Vector2f(0.1f, 0.0f), true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 80, 8, sf::Vector2f(2.0f, -8.0f), sf::Vector2f(0.3f, 1.0f), sf::Vector2f(0.1f, 0.0f), true));

	// recovery frames
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false, true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));

	return attack_frames;
}

std::vector<std::vector<AttackFrame*>> Attack::MakeMoveForwardSmash() {
	std::vector<std::vector<AttackFrame*>> attack_frames = std::vector<std::vector<AttackFrame*>>();

	// startup frames
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false, true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));

	// active frames
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 400, 3, sf::Vector2f(4.0f, 4.0f), sf::Vector2f(1.0f, 0.15f), sf::Vector2f(0.6f, -0.25f), true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 400, 6, sf::Vector2f(4.0f, 4.0f), sf::Vector2f(1.0f, 0.15f), sf::Vector2f(0.6f, -0.25f), true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 400, 3, sf::Vector2f(4.0f, 4.0f), sf::Vector2f(1.0f, 0.15f), sf::Vector2f(0.6f, -0.25f), true));

	// recovery frames
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false, true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));

	return attack_frames;
}

std::vector<std::vector<AttackFrame*>> Attack::MakeMoveForwardAir() {
	std::vector<std::vector<AttackFrame*>> attack_frames = std::vector<std::vector<AttackFrame*>>();

	// startup frames
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false, true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));

	// active frames
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 150, 30, sf::Vector2f(8.0f, -8.0f), sf::Vector2f(0.3f, 0.3f), sf::Vector2f(0.2f, 0.0f), true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 150, 30, sf::Vector2f(8.0f, -8.0f), sf::Vector2f(0.3f, 0.3f), sf::Vector2f(0.2f, 0.0f), true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 150, 30, sf::Vector2f(8.0f, -8.0f), sf::Vector2f(0.3f, 0.3f), sf::Vector2f(0.2f, 0.0f), true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 10, 10, sf::Vector2f(2.0f, -5.0f), sf::Vector2f(0.3f, 0.3f), sf::Vector2f(0.2f, 0.0f), true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 10, 10, sf::Vector2f(2.0f, -5.0f), sf::Vector2f(0.3f, 0.3f), sf::Vector2f(0.2f, 0.0f), true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 10, 10, sf::Vector2f(2.0f, -5.0f), sf::Vector2f(0.3f, 0.3f), sf::Vector2f(0.2f, 0.0f), true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 10, 10, sf::Vector2f(2.0f, -5.0f), sf::Vector2f(0.3f, 0.3f), sf::Vector2f(0.2f, 0.0f), true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 10, 10, sf::Vector2f(2.0f, -5.0f), sf::Vector2f(0.3f, 0.3f), sf::Vector2f(0.2f, 0.0f), true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 10, 10, sf::Vector2f(2.0f, -5.0f), sf::Vector2f(0.3f, 0.3f), sf::Vector2f(0.2f, 0.0f), true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 10, 10, sf::Vector2f(2.0f, -5.0f), sf::Vector2f(0.3f, 0.3f), sf::Vector2f(0.2f, 0.0f), true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 10, 10, sf::Vector2f(2.0f, -5.0f), sf::Vector2f(0.3f, 0.3f), sf::Vector2f(0.2f, 0.0f), true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 10, 10, sf::Vector2f(2.0f, -5.0f), sf::Vector2f(0.3f, 0.3f), sf::Vector2f(0.2f, 0.0f), true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 10, 10, sf::Vector2f(2.0f, -5.0f), sf::Vector2f(0.3f, 0.3f), sf::Vector2f(0.2f, 0.0f), true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 10, 10, sf::Vector2f(2.0f, -5.0f), sf::Vector2f(0.3f, 0.3f), sf::Vector2f(0.2f, 0.0f), true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 10, 10, sf::Vector2f(2.0f, -5.0f), sf::Vector2f(0.3f, 0.3f), sf::Vector2f(0.2f, 0.0f), true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 10, 10, sf::Vector2f(2.0f, -5.0f), sf::Vector2f(0.3f, 0.3f), sf::Vector2f(0.2f, 0.0f), true));

	// recovery frames
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false, true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));

	return attack_frames;
}

std::vector<std::vector<AttackFrame*>> Attack::MakeMoveBackAir() {
	std::vector<std::vector<AttackFrame*>> attack_frames = std::vector<std::vector<AttackFrame*>>();

	// startup frames
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false, true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));

	// active frames
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 10, 10, sf::Vector2f(2.0f, 5.0f), sf::Vector2f(0.4f, 0.2f), sf::Vector2f(-0.2f, 0.0f), true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 10, 10, sf::Vector2f(2.0f, 5.0f), sf::Vector2f(0.4f, 0.2f), sf::Vector2f(-0.2f, 0.0f), true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 10, 10, sf::Vector2f(2.0f, 5.0f), sf::Vector2f(0.4f, 0.2f), sf::Vector2f(-0.2f, 0.0f), true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 10, 10, sf::Vector2f(2.0f, 5.0f), sf::Vector2f(0.4f, 0.2f), sf::Vector2f(-0.2f, 0.0f), true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 10, 10, sf::Vector2f(2.0f, 5.0f), sf::Vector2f(0.4f, 0.2f), sf::Vector2f(-0.2f, 0.0f), true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 10, 10, sf::Vector2f(2.0f, 5.0f), sf::Vector2f(0.4f, 0.2f), sf::Vector2f(-0.2f, 0.0f), true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 10, 10, sf::Vector2f(2.0f, 5.0f), sf::Vector2f(0.4f, 0.2f), sf::Vector2f(-0.2f, 0.0f), true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 10, 10, sf::Vector2f(2.0f, 5.0f), sf::Vector2f(0.4f, 0.2f), sf::Vector2f(-0.2f, 0.0f), true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 10, 10, sf::Vector2f(2.0f, 5.0f), sf::Vector2f(0.4f, 0.2f), sf::Vector2f(-0.2f, 0.0f), true));

	// recovery frames
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false, true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));

	return attack_frames;
}

std::vector<std::vector<AttackFrame*>> Attack::MakeMoveDownAir() {
	std::vector<std::vector<AttackFrame*>> attack_frames = std::vector<std::vector<AttackFrame*>>();

	// startup frames
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false, true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));

	// active frames
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 20, 30, sf::Vector2f(0.0f, -12.0f), sf::Vector2f(0.8f, 0.75f), sf::Vector2f(0.0f, 0.5f), true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 20, 30, sf::Vector2f(0.0f, -12.0f), sf::Vector2f(0.8f, 0.75f), sf::Vector2f(0.0f, 0.5f), true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 20, 30, sf::Vector2f(0.0f, -12.0f), sf::Vector2f(0.8f, 0.75f), sf::Vector2f(0.0f, 0.5f), true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 20, 30, sf::Vector2f(0.0f, -12.0f), sf::Vector2f(0.8f, 0.75f), sf::Vector2f(0.0f, 0.5f), true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 20, 30, sf::Vector2f(0.0f, -12.0f), sf::Vector2f(0.8f, 0.75f), sf::Vector2f(0.0f, 0.5f), true));

	// recovery frames
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false, true));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));
	attack_frames.push_back(std::vector<AttackFrame*>()); attack_frames[(int)attack_frames.size() - 1].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false));

	return attack_frames;
}