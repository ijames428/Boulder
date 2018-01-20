#pragma once

using namespace std;
#include "stdafx.h"
#include <iostream>
#include "HurtBox.h"
#include "GameLibrary\Singleton.h"
#include "SmashWorld.h"
#include <vector>
#include <string>
#include <sstream>

#define DEGTORAD 0.0174532925199432957f

HurtBox::HurtBox(b2Body* body, int player_index, Json::Value jsonData) {
	float boxScale = 40.0f;

	std::vector<float> vect_hurt_box;
	//jsonData["DictOfUnits"][unit_type][animations_name][i]
	//string strHurtBoxValues = jsonData["HurtBoxPerFrame"][i][box]["Box"].asString();
	string strHurtBoxValues = jsonData["Box"].asString();

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
		vect_hurt_box.push_back(1.0f);
		vect_hurt_box.push_back(1.0f);
		vect_hurt_box.push_back(1.0f);
		vect_hurt_box.push_back(1.0f);
	}

	float hurt_box_relative_x = ((vect_hurt_box[0] + (vect_hurt_box[2] / 2.0f)) / boxScale);
	float hurt_box_relative_y = ((vect_hurt_box[1] + (vect_hurt_box[3] / 2.0f) - (vect_hurt_box[1] + (vect_hurt_box[3] / 2.0f))) / boxScale);// -(radius * 30.0f);
	float hurt_box_width = (vect_hurt_box[2]) / boxScale;
	float hurt_box_height = (vect_hurt_box[3]) / boxScale;

	//attack_frames[i].push_back(new AttackFrame(player_body, player_index, (int)attack_frames.size(), damage, frames_of_hurt_stun,
	//	sf::Vector2f(knockback_x, knockback_y), sf::Vector2f(hurt_box_width, hurt_box_height), sf::Vector2f(hurt_box_relative_x, hurt_box_relative_y),
	//	true));

	facing_right = true;

	polygonShapeLeft.SetAsBox(hurt_box_width / 2.0f, hurt_box_height / 2.0f, b2Vec2(-hurt_box_relative_x, hurt_box_relative_y), 0.0f);
	myFixtureDefLeft.shape = &polygonShapeLeft;
	myFixtureDefLeft.isSensor = true;
	myFixtureDefLeft.m_color = new b2Color(1.0f, 0.0f, 1.0f, 1.0f);

	if (player_index == 0) {
		myFixtureDefLeft.filter.categoryBits = Singleton<SmashWorld>::Get()->PLAYER_CHARACTER;
		myFixtureDefLeft.filter.maskBits = Singleton<SmashWorld>::Get()->HIT_BOX;
	} else {
		myFixtureDefLeft.filter.categoryBits = Singleton<SmashWorld>::Get()->ENEMY;
		myFixtureDefLeft.filter.maskBits = Singleton<SmashWorld>::Get()->HIT_BOX;
	}

	myFixtureLeft = body->CreateFixture(&myFixtureDefLeft);
	myFixtureLeft->SetActive(false);


	polygonShapeRight.SetAsBox(hurt_box_width / 2.0f, hurt_box_height / 2.0f, b2Vec2(hurt_box_relative_x, hurt_box_relative_y), 0.0f);
	myFixtureDefRight.shape = &polygonShapeRight;
	myFixtureDefRight.isSensor = true;
	myFixtureDefRight.m_color = new b2Color(1.0f, 0.0f, 1.0f, 1.0f);

	if (player_index == 0) {
		myFixtureDefRight.filter.categoryBits = Singleton<SmashWorld>::Get()->PLAYER_CHARACTER;
		myFixtureDefRight.filter.maskBits = Singleton<SmashWorld>::Get()->HIT_BOX;
	}
	else {
		myFixtureDefRight.filter.categoryBits = Singleton<SmashWorld>::Get()->ENEMY;
		myFixtureDefRight.filter.maskBits = Singleton<SmashWorld>::Get()->HIT_BOX;
	}

	myFixtureRight = body->CreateFixture(&myFixtureDefRight);
	myFixtureRight->SetActive(true);
}

void HurtBox::Update(bool new_facing_right) {
	if (new_facing_right && myFixtureLeft->IsActive()) {
		myFixtureLeft->SetActive(false);
		myFixtureRight->SetActive(true);
	} else if (!new_facing_right && myFixtureRight->IsActive()) {
		myFixtureRight->SetActive(false);
		myFixtureLeft->SetActive(true);
	}

	facing_right = new_facing_right;
}

bool HurtBox::IsFacingRight() {
	return facing_right;
}

void HurtBox::Activate() {
	if (facing_right) {
		myFixtureRight->SetActive(true);
	} else {
		myFixtureLeft->SetActive(true);
	}
}

void HurtBox::Deactivate() {
	if (facing_right) {
		myFixtureRight->SetActive(false);
	} else {
	}
}

b2Shape* HurtBox::GetShape() {
	if (facing_right) {
		return myFixtureRight->GetShape();
	} else {
		return myFixtureLeft->GetShape();
	}
}