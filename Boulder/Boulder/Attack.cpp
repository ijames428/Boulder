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

std::vector<HitBox*> AttackFrame::ExistingHitBoxes;

Attack::Attack(b2Body* body, int index, int move_type, Json::Value jsonData) {
	player_body = body;
	player_index = index;
	float boxScale = 40.0f;

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
					int frames_of_hit_stun = jsonData["HitBoxPerFrame"][i][box]["HitStunFrames"].asInt();
					bool pop_up_move = jsonData["HitBoxPerFrame"][i][box]["PopUp"].asBool();

					std::vector<float> vect_hurt_box;
					string strHurtBoxValues = jsonData["HurtBoxPerFrame"][0][box]["Box"].asString();

					if (strHurtBoxValues != "") {
						std::stringstream ss_hurt_box(strHurtBoxValues);
						float hurt_box_character;

						while (ss_hurt_box >> hurt_box_character)
						{
							vect_hurt_box.push_back(hurt_box_character);

							if (ss_hurt_box.peek() == ',')
								ss_hurt_box.ignore();
						}
					} else {
						vect_hurt_box.push_back(0.0f);
						vect_hurt_box.push_back(0.0f);
						vect_hurt_box.push_back(0.0f);
						vect_hurt_box.push_back(0.0f);
					}

					std::vector<float> vect_hit_box;
					string strHitBoxValues = jsonData["HitBoxPerFrame"][i][box]["Box"].asString();
					std::stringstream ss_hit_box(strHitBoxValues);
					float hit_box_character;

					while (ss_hit_box >> hit_box_character)
					{
						vect_hit_box.push_back(hit_box_character);

						if (ss_hit_box.peek() == ',')
							ss_hit_box.ignore();
					}

					float radius = player_body->GetFixtureList()[0].GetShape()->m_radius;
					float hit_box_relative_x = ((vect_hit_box[0] + (vect_hit_box[2] / 2.0f)) / boxScale);
					float hit_box_relative_y = ((vect_hit_box[1] + (vect_hit_box[3] / 2.0f) - (vect_hurt_box[1] + (vect_hurt_box[3] / 2.0f))) / boxScale);// -(radius * 30.0f);
					float hit_box_width = (vect_hit_box[2]) / boxScale;
					float hit_box_height = (vect_hit_box[3]) / boxScale;

					attack_frames[i].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), damage, frames_of_hit_stun,
															   sf::Vector2f(knockback_x, knockback_y), sf::Vector2f(hit_box_width, hit_box_height), sf::Vector2f(hit_box_relative_x, hit_box_relative_y),
															   true, strHitBoxValues, pop_up_move));
				}
			}
			else
			{
				attack_frames[i].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), 0, 0, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), false, "", false, true));
			}
		}
	}

	attack_frames_size = (int)attack_frames.size();

	attack_timer = new StatusTimer(jsonData["NumberOfFrames"].asInt());

	enemies_hit = std::vector<string>();
}

void Attack::Update(sf::Uint64 curr_frame, bool facing_right) {
	current_frame = curr_frame;
	current_frame_in_attack = current_frame - starting_attack_frame;

	if (IsAttacking()) {
		//if (current_frame_in_attack > 0) {
		//	int last_attack_frame = (int)current_frame_in_attack - 1;
		//	int last_attack_frames_size = (int)attack_frames[last_attack_frame].size();
		//	for (int box = 0; box < last_attack_frames_size; box++) {
		//		if (attack_frames[last_attack_frame][box]->IsActive()) {
		//			attack_frames[last_attack_frame][box]->Deactivate();
		//		}
		//	}
		//}
		//
		//if (attack_frames_size > current_frame_in_attack)
		//{
		//	int this_attack_frames_size = (int)attack_frames[current_frame_in_attack].size();
		//	for (int box = 0; box < this_attack_frames_size; box++) {
		//		if (attack_frames[current_frame_in_attack][box]->IsClearingFrame()) {
		//			enemies_hit.clear();
		//		}
		//
		//		attack_frames[current_frame_in_attack][box]->Activate();
		//	}
		//}
		for (int i = 0; i < attack_frames_size; i++) {
			int this_attack_frames_size = (int)attack_frames[i].size();
			if (i == current_frame_in_attack) {
				for (int box = 0; box < this_attack_frames_size; box++) {
					attack_frames[i][box]->Update(facing_right);
					attack_frames[i][box]->Activate();
		
					if (attack_frames[i][box]->IsClearingFrame()) {
						enemies_hit.clear();
					}
				}
			} else {
				for (int box = 0; box < this_attack_frames_size; box++) {
					attack_frames[i][box]->Deactivate();
				}
			}
		}
	} else {
		for (int i = 0; i < attack_frames_size; i++) {
			int this_attack_frames_size = (int)attack_frames[i].size();
			for (int box = 0; box < this_attack_frames_size; box++) {
				attack_frames[i][box]->Update(facing_right);
			}
		}
	}

	//for (int i = 0; i < attack_frames_size; i++) {
	//	int this_attack_frames_size = (int)attack_frames[i].size();
	//	for (int box = 0; box < this_attack_frames_size; box++) {
	//		attack_frames[i][box]->Update(facing_right);
	//	}
	//}
}

void Attack::InitiateAttack() {
	current_frame_in_attack = 0;
	starting_attack_frame = current_frame;
	attack_timer->Start();
}

void Attack::StopAttack() {
	attack_timer->Stop();
	for (int i = 0; i < attack_frames_size; i++) {
		for (int box = 0; box < (int)attack_frames[i].size(); box++) {
			attack_frames[i][box]->ScheduleDeactivate();
		}
	}
	enemies_hit.clear();
}

int Attack::GetDamage() {
	if (current_frame_in_attack >= attack_frames_size) {
		return 0;
	}

	// TODO: Separate out the boxes so we can get the damage of an individual one.
	return attack_frames[(int)current_frame_in_attack][0]->GetDamage();
}

bool Attack::IsAttacking() {
	return attack_timer->IsActive();
}

sf::Vector2f Attack::GetKnockBack() {
	if (current_frame_in_attack >= attack_frames_size) {
		return sf::Vector2f(0.0f, 0.0f);
	}

	// TODO: Separate out the boxes so we can get the knockback of an individual one.
	return attack_frames[(int)current_frame_in_attack][0]->GetKnockBack();
}

int Attack::GetHitStunFrames() {
	if (current_frame_in_attack >= attack_frames_size) {
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


AttackFrame::AttackFrame(b2Body* body, int player_index, int frame_in_attack, int dmg, sf::Int64 stun_frames, sf::Vector2f kfr, sf::Vector2f hit_box_dimensions, sf::Vector2f hit_box_relative_central_position, bool has_hb, string box_info, bool pop_up_grounded_enemies, bool clear_hit_enemies) {
	has_hit_box = has_hb;
	frame_for_clearing_enemies_hit = clear_hit_enemies;

	if (has_hit_box) {
		HitBox* existing_hit_box = AttackFrame::GetExistingHitBox(box_info);

		if (existing_hit_box == nullptr) {
			hitbox = new HitBox(body, player_index, frame_in_attack, hit_box_dimensions, hit_box_relative_central_position, box_info, pop_up_grounded_enemies);
		} else {
			hitbox = existing_hit_box;
		}
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

void AttackFrame::ScheduleDeactivate() {
	if (HasHitBox()) {
		hitbox->ScheduleDeactivate();
	}
}

bool AttackFrame::IsActive() {
	if (HasHitBox()) {
		return hitbox->IsActive();
	}

	return false;
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

HitBox* AttackFrame::GetExistingHitBox(string box_info) {
	for (int i = 0; i < (int)AttackFrame::ExistingHitBoxes.size(); i++) {
		if (AttackFrame::ExistingHitBoxes[i]->BoxInfo == box_info) {
			return AttackFrame::ExistingHitBoxes[i];
		}
	}

	return nullptr;
}


HitBox::HitBox(b2Body* bod, int player_index, int frame_in_attack, sf::Vector2f dimensions, sf::Vector2f relative_central_position, string box_info, bool pop_up_grounded_enemies) {
	facing_right = true;
	BoxInfo = box_info;
	body = bod;
	deactivateOnUpdate = false;
	popUpGroundedEnemies = pop_up_grounded_enemies;

	polygonShapeLeft.SetAsBox(dimensions.x / 2.0f, dimensions.y / 2.0f, b2Vec2(-relative_central_position.x, relative_central_position.y), 0.0f);
	myFixtureDefLeft.shape = &polygonShapeLeft;
	myFixtureDefLeft.isSensor = true;
	myFixtureDefLeft.m_color = new b2Color(1.0f, 0.0f, 1.0f, 1.0f);
	
	if (player_index == 0) {
		myFixtureDefLeft.filter.categoryBits = Singleton<SmashWorld>::Get()->HIT_BOX;
		myFixtureDefLeft.filter.maskBits = Singleton<SmashWorld>::Get()->ENEMY | Singleton<SmashWorld>::Get()->DOOR;
	} else {
		myFixtureDefLeft.filter.categoryBits = Singleton<SmashWorld>::Get()->HIT_BOX;
		myFixtureDefLeft.filter.maskBits = Singleton<SmashWorld>::Get()->PLAYER_CHARACTER;
	}
	
	//myFixtureLeft = body->CreateFixture(&myFixtureDefLeft);
	//myFixtureLeft->SetActive(false);


	polygonShapeRight.SetAsBox(dimensions.x / 2.0f, dimensions.y / 2.0f, b2Vec2(relative_central_position.x, relative_central_position.y), 0.0f);
	myFixtureDefRight.shape = &polygonShapeRight;
	myFixtureDefRight.isSensor = true;
	myFixtureDefRight.m_color = new b2Color(1.0f, 0.0f, 1.0f, 1.0f);
	
	if (player_index == 0) {
		myFixtureDefRight.filter.categoryBits = Singleton<SmashWorld>::Get()->HIT_BOX;
		myFixtureDefRight.filter.maskBits = Singleton<SmashWorld>::Get()->ENEMY | Singleton<SmashWorld>::Get()->DOOR;
	} else {
		myFixtureDefRight.filter.categoryBits = Singleton<SmashWorld>::Get()->HIT_BOX;
		myFixtureDefRight.filter.maskBits = Singleton<SmashWorld>::Get()->PLAYER_CHARACTER;
	}
	
	//myFixtureRight = body->CreateFixture(&myFixtureDefRight);
	//myFixtureRight->SetActive(false);
}

void HitBox::Update(bool new_facing_right) {
	if (deactivateOnUpdate) {
		Deactivate();
	}
	facing_right = new_facing_right;
}

bool HitBox::IsFacingRight() {
	return facing_right;
}

void HitBox::Activate() {
	if (facing_right) {
		myFixtureRight = body->CreateFixture(&myFixtureDefRight);
		myFixtureRight->SetActive(true);
	} else {
		myFixtureLeft = body->CreateFixture(&myFixtureDefLeft);
		myFixtureLeft->SetActive(true);
	}
}

void HitBox::Deactivate() {
	if (myFixtureRight != nullptr) {
		body->DestroyFixture(myFixtureRight);
		myFixtureRight = nullptr;
	}

	if (myFixtureLeft != nullptr) {
		body->DestroyFixture(myFixtureLeft);
		myFixtureLeft = nullptr;
	}
}

void HitBox::ScheduleDeactivate() {
	deactivateOnUpdate = true;
}

bool HitBox::IsActive() {
	if (facing_right) {
		return myFixtureRight != nullptr;
	} else {
		return myFixtureLeft != nullptr;
	}
}

bool HitBox::IsPopUpMove() {
	return popUpGroundedEnemies;
}

bool AttackFrame::IsPopUpMove() {
	if (HasHitBox()) {
		return hitbox->IsPopUpMove();
	}

	return false;
}

bool Attack::IsPopUpMove() {
	if (current_frame_in_attack >= attack_frames_size) {
		return 0;
	}

	// TODO: Separate out the boxes so we can get the damage of an individual one.
	return attack_frames[(int)current_frame_in_attack][0]->IsPopUpMove();
}