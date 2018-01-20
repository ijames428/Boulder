#pragma once

using namespace std;
#include "stdafx.h"
#include <iostream>
#include <sstream>
#include "BoulderCreature.h"
#include "Constants.h"
#include "SmashWorld.h"
#include <cmath>

bool Contains(string string_being_searched, string string_being_searched_for) {
	std::size_t found = string_being_searched.find(string_being_searched_for);
	return found != std::string::npos;
}

BoulderCreature::BoulderCreature(int player_idx, sf::RenderWindow *window, sf::Vector2f position, sf::Vector2f dimensions, bool subject_to_gravity) :
	Creature::Creature(window, position, dimensions, subject_to_gravity) {
}

BoulderCreature::BoulderCreature(string unit_name, string unit_type, string bestiary_name, bool is_npc, Json::Value jsonBestiariesData, sf::RenderWindow *window, sf::Vector2f position, sf::Vector2f dimensions, bool subject_to_gravity) :
	Creature::Creature(window, position, dimensions, subject_to_gravity) {
	SetEntityType(Constants::ENTITY_TYPE_CREATURE);
	name = unit_name;
	type = unit_type;
	movement = 0.0f;
	SetFacingRight(true);
	player_index = 1;

	cashinable_hit_point_value = hit_points = max_hit_points = jsonBestiariesData["DictOfUnits"][unit_type]["HitPoints"].asInt();
	interaction_radius = jsonBestiariesData["DictOfUnits"][unit_type]["InteractionRadius"].asFloat();
	can_take_input = false;

	cashinable_hit_point_drain_rate = 2;

	speed = jsonBestiariesData["DictOfUnits"][unit_type]["MovementSpeed"].asFloat();
	running_speed_multiplier = 3.0f;
	jump_power = 10.0f;

	is_interactable = is_npc;
	is_hittable = !is_interactable;
	is_hostile = !is_interactable;

	sf::RectangleShape shape(dimensions);
	shape.setFillColor(sf::Color::Yellow);
	shape.setPosition(position);
	rectangle_shape = shape;
	player_color = sf::Color::Cyan;

	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(position.x, position.y);
	body = Singleton<SmashWorld>::Get()->GetB2World()->CreateBody(&bodyDef);
	
	starting_health_bar_width = 40.0f;
	healthBarRect = new sf::RectangleShape(sf::Vector2f(starting_health_bar_width, 5.0f));
	healthBarRect->setPosition(sf::Vector2f(body->GetPosition().x * 40.0f, body->GetPosition().y * 40.0f));
	healthBarRect->setFillColor(sf::Color::Green);

	cashInHealthBarRect = new sf::RectangleShape(sf::Vector2f(starting_health_bar_width, 5.0f));
	cashInHealthBarRect->setPosition(sf::Vector2f(body->GetPosition().x * 40.0f, body->GetPosition().y * 40.0f));
	cashInHealthBarRect->setFillColor(sf::Color(255, 127, 127, 255));

	aggroCircleShape.m_p.Set(0, 0); //position, relative to body position
	aggroCircleShape.m_radius = 3.0f;
	aggroCircleFixtureDef.shape = &aggroCircleShape;
	aggroCircleFixtureDef.isSensor = true;
	aggroCircleFixtureDef.m_color = new b2Color(1.0f, 1.0f, 1.0f, 1.0f);
	aggroCircleFixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->AGGRO_CIRCLE;
	aggroCircleFixtureDef.filter.maskBits = Singleton<SmashWorld>::Get()->PLAYER_CHARACTER;

	deaggroCircleShape.m_p.Set(0, 0); //position, relative to body position
	deaggroCircleShape.m_radius = 5.0f;
	deaggroCircleFixtureDef.shape = &deaggroCircleShape;
	deaggroCircleFixtureDef.isSensor = true;
	deaggroCircleFixtureDef.m_color = new b2Color(1.0f, 1.0f, 1.0f, 1.0f);
	deaggroCircleFixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->DEAGGRO_CIRCLE;
	deaggroCircleFixtureDef.filter.maskBits = Singleton<SmashWorld>::Get()->PLAYER_CHARACTER;

	interactionCircleShape.m_p.Set(0, 0); //position, relative to body position
	interactionCircleShape.m_radius = interaction_radius / 100.0f;
	interactionCircleFixtureDef.shape = &interactionCircleShape;
	interactionCircleFixtureDef.isSensor = true;
	interactionCircleFixtureDef.m_color = new b2Color(1.0f, 1.0f, 0.0f, 1.0f);
	interactionCircleFixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->INTERACTION_CIRCLE;
	interactionCircleFixtureDef.filter.maskBits = Singleton<SmashWorld>::Get()->PLAYER_CHARACTER;

	std::vector<float> vect_hurt_box;
	string strHurtBoxValues = jsonBestiariesData["DictOfUnits"][unit_type]["IdleAnimations"][0]["HurtBoxPerFrame"][0][0]["Box"].asString();
	std::stringstream ss_hurt_box;

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

	centerBoxShape.SetAsBox(vect_hurt_box[2] / 160.0f, vect_hurt_box[3] / 100.0f);
	topCircleShape.m_p.Set(0, -vect_hurt_box[3] / 100.0f); //position, relative to body position
	botCircleShape.m_p.Set(0, vect_hurt_box[3] / 100.0f); //position, relative to body position
	topCircleShape.m_radius = vect_hurt_box[2] / 160.0f;
	botCircleShape.m_radius = vect_hurt_box[2] / 160.0f;
	centerBoxFixtureDef.shape = &centerBoxShape;
	centerBoxFixtureDef.density = 1.0f;
	centerBoxFixtureDef.friction = 0.0f;
	centerBoxFixtureDef.m_color = new b2Color(0.0f, 1.0f, 0.0f, 1.0f);
	topCircleFixtureDef.shape = &topCircleShape;
	topCircleFixtureDef.density = 1.0f;
	topCircleFixtureDef.friction = 0.01f;
	topCircleFixtureDef.m_color = new b2Color(0.0f, 1.0f, 0.0f, 1.0f);
	botCircleFixtureDef.shape = &botCircleShape;
	botCircleFixtureDef.density = 1.0f;
	botCircleFixtureDef.friction = 0.3f;
	botCircleFixtureDef.m_color = new b2Color(0.0f, 1.0f, 0.0f, 1.0f);

	default_masked_bits = centerBoxFixtureDef.filter.maskBits;

	topCircleFixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->ENEMY;
	botCircleFixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->ENEMY;
	centerBoxFixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->ENEMY;

	aggroCircleFixture = body->CreateFixture(&aggroCircleFixtureDef);
	deaggroCircleFixture = body->CreateFixture(&deaggroCircleFixtureDef);
	interactionCircleFixture = body->CreateFixture(&interactionCircleFixtureDef);
	topCircleFixture = body->CreateFixture(&topCircleFixtureDef);
	botCircleFixture = body->CreateFixture(&botCircleFixtureDef);
	centerBoxFixture = body->CreateFixture(&centerBoxFixtureDef);

	//interactionCircleFixture->SetActive(is_interactable && interaction_radius > 0.0f);

	if (is_interactable) {
		topCircleFixture->SetSensor(true);
		botCircleFixture->SetSensor(true);
		centerBoxFixture->SetSensor(true);

		body->SetGravityScale(0.0f);
	}

	hurt_box = new HurtBox(body, player_index, jsonBestiariesData["DictOfUnits"][unit_type]["IdleAnimations"][0]["HurtBoxPerFrame"][0][0]);

	body->SetFixedRotation(true);
	body->SetUserData(this);

	groundCheckShape.SetAsBox(vect_hurt_box[2] / 100.0f, 0.05f, b2Vec2(0.0f, (vect_hurt_box[3] / 100.0f) + (vect_hurt_box[2] / 160.0f)), 0.0f);
	groundCheckFixtureDef.shape = &groundCheckShape;
	groundCheckFixtureDef.isSensor = true;
	groundCheckFixtureDef.m_color = new b2Color(1.0f, 0.0f, 1.0f, 1.0f);

	groundCheckFixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->GROUND_CHECK;
	groundCheckFixtureDef.filter.maskBits = Singleton<SmashWorld>::Get()->PLATFORM;

	body->CreateFixture(&groundCheckFixtureDef);

	attacks.push_back(new Attack(body, player_index, Attack::FORWARD_SMASH, jsonBestiariesData["DictOfUnits"][unit_type]["AttackingAnimations"][0]));
	attacks.push_back(new Attack(body, player_index, Attack::DOWN_SMASH, jsonBestiariesData["DictOfUnits"][unit_type]["AttackingAnimations"][1]));

	sprite_scale = 1.0f;

	RightFootStepSoundFrameWalk = 0;
	LeftFootStepSoundFrameWalk = 0;
	RightFootStepSoundFrameRun = 0;
	LeftFootStepSoundFrameRun = 0;

	GettingHitSoundBuffers = std::vector<sf::SoundBuffer*>();
	GettingHitSounds = std::vector<sf::Sound*>();

	for (int i = 0; i < (int)jsonBestiariesData["DictOfUnits"][unit_type]["GettingHitSoundFiles"].size(); i++) {
		string soundFilePath = jsonBestiariesData["DictOfUnits"][unit_type]["GettingHitSoundFiles"][i].asString();
		size_t findResult = soundFilePath.find("Sound\\");
		string relativeFilePath = soundFilePath.substr(findResult);

		GettingHitSoundBuffers.push_back(new sf::SoundBuffer());
		GettingHitSoundBuffers[i]->loadFromFile(relativeFilePath);
		GettingHitSounds.push_back(new sf::Sound());
		GettingHitSounds[i]->setBuffer(*GettingHitSoundBuffers[i]);
	}

	LoadAllAnimations(unit_type, jsonBestiariesData);

	hit_stun_timer = new StatusTimer(1);
	jump_input_buffer = new StatusTimer(6);
	health_cash_in_timer = new StatusTimer(360);

	int numberOfDyingAnimationFrames = dying_animations.size() > 0 ? dying_animations[0]->GetNumberOfFrames() : 0;
	dying_animation_timer = new StatusTimer(numberOfDyingAnimationFrames);

	int numberOfLandingAnimationFrames = landing_animations.size() > 0 ? landing_animations[0]->GetNumberOfFrames() : 1;
	landing_animation_timer = new StatusTimer(numberOfLandingAnimationFrames);

	talking_animation_timer = new StatusTimer(120);
}

void BoulderCreature::ApplyObjectDataToSaveData(Json::Value& save_data) {
	//save_data["Name"] = name;
	save_data["PositionX"] = body->GetPosition().x;
	save_data["PositionY"] = body->GetPosition().y;
	save_data["CurrentHitPoints"] = hit_points;
}

void BoulderCreature::ApplySaveDataToObjectData(Json::Value& save_data) {
	body->SetTransform(b2Vec2(save_data["PositionX"].asFloat(), save_data["PositionY"].asFloat()), body->GetAngle());
	hit_points = save_data["CurrentHitPoints"].asInt();
}

void BoulderCreature::LoadAllAnimations(string unit_type, Json::Value jsonBestiariesData) {
	idle_animations = LoadAnimations("IdleAnimations", unit_type, jsonBestiariesData);
	walking_animations = LoadAnimations("WalkingAnimations", unit_type, jsonBestiariesData);
	running_animations = LoadAnimations("RunningAnimations", unit_type, jsonBestiariesData);
	dying_animations = LoadAnimations("DyingAnimations", unit_type, jsonBestiariesData);
	dead_animations = LoadAnimations("DeadAnimations", unit_type, jsonBestiariesData);
	attacking_animations = LoadAnimations("AttackingAnimations", unit_type, jsonBestiariesData);
	blocking_animations = LoadAnimations("BlockingAnimations", unit_type, jsonBestiariesData);
	hit_stun_animations = LoadAnimations("HitStunAnimations", unit_type, jsonBestiariesData);
	jumping_animations = LoadAnimations("JumpingAnimations", unit_type, jsonBestiariesData);
	jump_apex_animations = LoadAnimations("JumpApexAnimations", unit_type, jsonBestiariesData);
	falling_animations = LoadAnimations("FallingAnimations", unit_type, jsonBestiariesData);
	landing_animations = LoadAnimations("LandingAnimations", unit_type, jsonBestiariesData);
	talking_animations = LoadAnimations("TalkingAnimations", unit_type, jsonBestiariesData);
}

std::vector<SpriteAnimation*> BoulderCreature::LoadAnimations(string animations_name, string unit_type, Json::Value jsonData) {
	std::vector<SpriteAnimation*> animations = std::vector<SpriteAnimation*>();
	int number_of_animations = (int)jsonData["DictOfUnits"][unit_type][animations_name].size();

	for (int i = 0; i < number_of_animations; i++)
	{
		string filePath = jsonData["DictOfUnits"][unit_type][animations_name][i]["FilePath"].asString();

		if (filePath != "") {
			size_t findResult = filePath.find("Units\\");
			string relativeFilePath = filePath.substr(findResult);

			animations.push_back(new SpriteAnimation(render_window,
				relativeFilePath,
				jsonData["DictOfUnits"][unit_type][animations_name][i]["FrameDimensionsX"].asInt(),
				jsonData["DictOfUnits"][unit_type][animations_name][i]["FrameDimensionsY"].asInt(),
				jsonData["DictOfUnits"][unit_type][animations_name][i]["NumberOfFrames"].asInt(),
				jsonData["DictOfUnits"][unit_type][animations_name][i]["FramesPerColumn"].asInt(),
				jsonData["DictOfUnits"][unit_type][animations_name][i]["FramesPerRow"].asInt(),
				sprite_scale, sf::Color::White, animations_name != "JumpingAnimations" && animations_name != "JumpApexAnimations"));
		}

		if (jsonData["DictOfUnits"][unit_type][animations_name][i]["AttackSoundFilePerFrame"].size() > 0) {
			for (int f = 0; f < (int)jsonData["DictOfUnits"][unit_type][animations_name][i]["AttackSoundFilePerFrame"].size(); f++) {
				if (jsonData["DictOfUnits"][unit_type][animations_name][i]["AttackSoundFilePerFrame"][f].asString() != "") {
					string soundFilePath = jsonData["DictOfUnits"][unit_type][animations_name][i]["AttackSoundFilePerFrame"][f].asString();
					size_t findResult = soundFilePath.find("Sound\\");
					string relativeFilePath = soundFilePath.substr(findResult);

					AttackAnimationSoundFrames.push_back(f);

					AttackAnimationSoundBuffers.push_back(new sf::SoundBuffer());
					AttackAnimationSoundBuffers[i]->loadFromFile(relativeFilePath);
					AttackAnimationSounds.push_back(new sf::Sound());
					AttackAnimationSounds[i]->setBuffer(*AttackAnimationSoundBuffers[i]);

					break;
				}
			}
		}

		if (jsonData["DictOfUnits"][unit_type][animations_name][i]["RightFootSoundFilePerFrame"].size() > 0) {
			for (int f = 0; f < (int)jsonData["DictOfUnits"][unit_type][animations_name][i]["RightFootSoundFilePerFrame"].size(); f++) {
				if (jsonData["DictOfUnits"][unit_type][animations_name][i]["RightFootSoundFilePerFrame"][f].asString() != "") {
					string soundFilePath = jsonData["DictOfUnits"][unit_type][animations_name][i]["RightFootSoundFilePerFrame"][f].asString();
					size_t findResult = soundFilePath.find("Sound\\");
					string relativeFilePath = soundFilePath.substr(findResult);

					RightFootStepSoundBuffer.loadFromFile(relativeFilePath);
					RightFootStepSound.setBuffer(RightFootStepSoundBuffer);

					std::size_t found = animations_name.find("Walk");
					if (found != std::string::npos) {
						RightFootStepSoundFrameWalk = f;
					} else {
						RightFootStepSoundFrameRun = f;
					}

					break;
				}
			}
		}

		if (jsonData["DictOfUnits"][unit_type][animations_name][i]["LeftFootSoundFilePerFrame"].size() > 0) {
			for (int f = 0; f < (int)jsonData["DictOfUnits"][unit_type][animations_name][i]["LeftFootSoundFilePerFrame"].size(); f++) {
				if (jsonData["DictOfUnits"][unit_type][animations_name][i]["LeftFootSoundFilePerFrame"][f].asString() != "") {
					string soundFilePath = jsonData["DictOfUnits"][unit_type][animations_name][i]["LeftFootSoundFilePerFrame"][f].asString();
					size_t findResult = soundFilePath.find("Sound\\");
					string relativeFilePath = soundFilePath.substr(findResult);

					LeftFootStepSoundBuffer.loadFromFile(relativeFilePath);
					LeftFootStepSound.setBuffer(LeftFootStepSoundBuffer);

					std::size_t found = animations_name.find("Walk");
					if (found != std::string::npos) {
						LeftFootStepSoundFrameWalk = f;
					} else {
						LeftFootStepSoundFrameRun = f;
					}

					break;
				}
			}
		}
	}

	return animations;
}

void BoulderCreature::Update(sf::Int64 curr_frame, sf::Int64 delta_time) {
	Creature::Update(curr_frame, delta_time);

	can_take_input = !hit_stun_timer->IsActive();
	if (can_take_input) {
		can_take_input = !IsAnAttackActive();
	}

	hurt_box->Update(IsFacingRight());

	if (!health_cash_in_timer->IsActive()) {
		if (cashinable_hit_point_value > hit_points) {
			cashinable_hit_point_value -= cashinable_hit_point_drain_rate;

			if (cashinable_hit_point_value < hit_points) {
				cashinable_hit_point_value = hit_points;
			}
		}
	}

	if (hit_points <= 0 && centerBoxFixtureDef.filter.maskBits == default_masked_bits) {
		centerBoxFixtureDef.filter.maskBits = Singleton<SmashWorld>::Get()->PLATFORM;
		topCircleFixtureDef.filter.maskBits = Singleton<SmashWorld>::Get()->PLATFORM;
		botCircleFixtureDef.filter.maskBits = Singleton<SmashWorld>::Get()->PLATFORM;
		centerBoxFixture->SetFilterData(centerBoxFixtureDef.filter);
		topCircleFixture->SetFilterData(topCircleFixtureDef.filter);
		botCircleFixture->SetFilterData(botCircleFixtureDef.filter);
	} else if (hit_points > 0 && centerBoxFixtureDef.filter.maskBits == Singleton<SmashWorld>::Get()->PLATFORM) {
		centerBoxFixtureDef.filter.maskBits = default_masked_bits;
		topCircleFixtureDef.filter.maskBits = default_masked_bits;
		botCircleFixtureDef.filter.maskBits = default_masked_bits;
		centerBoxFixture->SetFilterData(centerBoxFixtureDef.filter);
		topCircleFixture->SetFilterData(topCircleFixtureDef.filter);
		botCircleFixture->SetFilterData(botCircleFixtureDef.filter);
	}

	for (int i = 0; i < (int)attacks.size(); i++) {
		attacks[i]->Update(curr_frame, IsFacingRight());
	}

	if (hit_stun_timer->IsActive()) {
		topCircleFixture->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
		botCircleFixture->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
		centerBoxFixture->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
	}
	else {
		topCircleFixture->SetColor(0.0f, 1.0f, 0.0f, 1.0f);
		botCircleFixture->SetColor(0.0f, 1.0f, 0.0f, 1.0f);
		centerBoxFixture->SetColor(0.0f, 1.0f, 0.0f, 1.0f);
	}

	UpdateBehavior();
	
	if (State == STATE_WALKING) {
		if (walking_animations[0]->GetCurrentFrame() == RightFootStepSoundFrameWalk) {
			RightFootStepSound.play();
		} else if (walking_animations[0]->GetCurrentFrame() == LeftFootStepSoundFrameWalk) {
			LeftFootStepSound.play();
		}
	} else if (State == STATE_RUNNING) {
		if (walking_animations[0]->GetCurrentFrame() == RightFootStepSoundFrameRun) {
			RightFootStepSound.play();
		} else if (walking_animations[0]->GetCurrentFrame() == LeftFootStepSoundFrameRun) {
			LeftFootStepSound.play();
		}
	}
}

void BoulderCreature::UpdateBehavior() {
	if (target != nullptr)
	{
		float delta_x = target->GetBody()->GetPosition().x - body->GetPosition().x;
		float delta_y = target->GetBody()->GetPosition().y - body->GetPosition().y;

		float distance = sqrtf(powf(delta_x, 2) + powf(delta_y, 2));

		if (distance < 2.0f) {
			if (!IsAnAttackActive()) {
				UseAttack(0);
			}
		}
		else {
			if (target->GetBody()->GetPosition().x < body->GetPosition().x)
			{
				movement = -100.0f;
				running = true;
			}
			else if (target->GetBody()->GetPosition().x > body->GetPosition().x)
			{
				movement = 100.0f;
				running = true;
			}
		}
	}
	else if (current_frame % 60 == 0) {
		int rng = rand() % 5;

		if (rng == 0) {
			movement = -100.0f;
			running = false;
		}
		else if (rng == 1) {
			movement = 0.0f;
			running = false;
		}
		else if (rng == 2) {
			movement = 100.0f;
			running = false;
		}
	}

	if (is_interactable) {
		body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));

		if (talking_animation_timer->IsActive()) {
			State = STATE_TALKING;
		}
		else {
			State = STATE_IDLE;
		}
	}
	else {
		Move(movement, 0.0f);

		if (hit_points <= 0) {
			if (dying_animation_timer->IsActive()) {
				State = STATE_DYING;
			}
			else {
				State = STATE_DEAD;
			}
		}
		else if (hit_stun_timer->IsActive()) {
			State = STATE_HIT_STUN;
		}
		else if (IsAnAttackActive()) {
			State = STATE_ATTACKING;
		}
		else if (body->GetLinearVelocity().x != 0) {
			if (running) {
				State = STATE_RUNNING;
			}
			else {
				State = STATE_WALKING;
			}
		}
		else {
			State = STATE_IDLE;
		}
	}
}

void BoulderCreature::Draw(sf::Vector2f camera_position) {
	//if (body->GetLinearVelocity().x > 0 && !IsFacingRight()) {
	//	SetFacingRight(true);
	//} else if (body->GetLinearVelocity().x < 0 && IsFacingRight()) {
	//	SetFacingRight(false);
	//}
	FlipAnimationsIfNecessary(idle_animations);
	FlipAnimationsIfNecessary(walking_animations);
	FlipAnimationsIfNecessary(running_animations);
	FlipAnimationsIfNecessary(dying_animations);
	FlipAnimationsIfNecessary(dead_animations);
	FlipAnimationsIfNecessary(attacking_animations);
	FlipAnimationsIfNecessary(blocking_animations);
	FlipAnimationsIfNecessary(hit_stun_animations);
	FlipAnimationsIfNecessary(jumping_animations);
	FlipAnimationsIfNecessary(jump_apex_animations);
	FlipAnimationsIfNecessary(falling_animations);
	FlipAnimationsIfNecessary(landing_animations);
	FlipAnimationsIfNecessary(talking_animations);

	float half_height = ((b2PolygonShape*)centerBoxFixture->GetShape())->m_vertices[3].y + botCircleShape.m_radius;// - ((b2PolygonShape*)centerBoxFixture->GetShape())->m_vertices[3].y;

	if (State == STATE_IDLE) {
		idle_animations[0]->Draw(camera_position, sf::Vector2f((body->GetPosition().x), (body->GetPosition().y)), half_height);
	}else if (State == STATE_WALKING) {
		walking_animations[0]->Draw(camera_position, sf::Vector2f((body->GetPosition().x), (body->GetPosition().y)), half_height);
	} else  if (State == STATE_RUNNING) {
		running_animations[0]->Draw(camera_position, sf::Vector2f((body->GetPosition().x), (body->GetPosition().y)), half_height);
	} else if (State == STATE_DYING) {
		if ((int)dying_animations.size() > 0) {
			dying_animations[0]->Draw(camera_position, sf::Vector2f((body->GetPosition().x), (body->GetPosition().y)), half_height);
		}
	} else if (State == STATE_DEAD) {
		if ((int)dead_animations.size() > 0) {
			//if (centerBoxFixture->GetShape()->m_type == b2Shape::e_polygon) {
				float half_height = ((b2PolygonShape*)centerBoxFixture->GetShape())->m_vertices[3].y;// - ((b2PolygonShape*)centerBoxFixture->GetShape())->m_vertices[3].y;
				dead_animations[0]->Draw(camera_position, sf::Vector2f((body->GetPosition().x), (body->GetPosition().y)), half_height);
			//} else {
			//	dead_animations[0]->Draw(camera_position, sf::Vector2f((body->GetPosition().x), (body->GetPosition().y)));
			//}
		}
	} else if (State == STATE_ATTACKING) {
		int currentAttackIndex = GetActiveAttackIndex();
		if (currentAttackIndex < (int)attacking_animations.size()) {
			attacking_animations[currentAttackIndex]->Draw(camera_position, sf::Vector2f((body->GetPosition().x), (body->GetPosition().y)), half_height);
		} else {
			attacking_animations[1]->Draw(camera_position, sf::Vector2f((body->GetPosition().x), (body->GetPosition().y)), half_height);
		}
	} else if (State == STATE_BLOCKING) {
		blocking_animations[0]->Draw(camera_position, sf::Vector2f((body->GetPosition().x), (body->GetPosition().y)), half_height);
	} else if (State == STATE_HIT_STUN) {
		hit_stun_animations[0]->Draw(camera_position, sf::Vector2f((body->GetPosition().x), (body->GetPosition().y)), half_height);
	} else if (State == STATE_JUMPING) {
		jumping_animations[0]->Draw(camera_position, sf::Vector2f((body->GetPosition().x), (body->GetPosition().y)), half_height);
	} else if (State == STATE_JUMP_APEX) {
		jump_apex_animations[0]->Draw(camera_position, sf::Vector2f((body->GetPosition().x), (body->GetPosition().y)), half_height);
	} else if (State == STATE_FALLING) {
		falling_animations[0]->Draw(camera_position, sf::Vector2f((body->GetPosition().x), (body->GetPosition().y)), half_height);
	} else if (State == STATE_LANDING) {
		if ((int)landing_animations.size() > 0) {
			landing_animations[0]->Draw(camera_position, sf::Vector2f((body->GetPosition().x), (body->GetPosition().y)), half_height);
		}
	} else if (State == STATE_TALKING) {
		if ((int)talking_animations.size() > 0) {
			talking_animations[0]->Draw(camera_position, sf::Vector2f((body->GetPosition().x), (body->GetPosition().y)), half_height);
		}
	}

	std::size_t found = name.find("Player");
	if (found == std::string::npos) {
		float percent_health = ((float)hit_points / (float)max_hit_points);

		healthBarRect->setSize(sf::Vector2f(starting_health_bar_width * percent_health, 5.0f));
		healthBarRect->setPosition(sf::Vector2f((body->GetPosition().x - 0.5f - camera_position.x) * 40.0f, (body->GetPosition().y - 0.6f - camera_position.y) * 40.0f));
		healthBarRect->setFillColor(sf::Color((int)(0.0f + (255.0f - (float)hit_points / (float)max_hit_points) * 255.0f), (int)((float)hit_points / (float)max_hit_points * 255.0f), 0, 255));

		float percent_cash_in_health = ((float)cashinable_hit_point_value / (float)max_hit_points);

		cashInHealthBarRect->setSize(sf::Vector2f(starting_health_bar_width * percent_cash_in_health, 5.0f));
		cashInHealthBarRect->setPosition(sf::Vector2f((body->GetPosition().x - 0.5f - camera_position.x) * 40.0f, (body->GetPosition().y - 0.6f - camera_position.y) * 40.0f));

		render_window->draw(*cashInHealthBarRect);
	}

	render_window->draw(*healthBarRect);
}

void BoulderCreature::AddActivaty(string activity) {
	if (activity != "") {
		if (std::find(activities.begin(), activities.end(), activity) == activities.end()) {
			activities.push_back(activity);
		}
	}
}

void BoulderCreature::FlipAnimationsIfNecessary(std::vector<SpriteAnimation*> animations)
{
	for (int i = 0; i < (int)animations.size(); i++) {
		if (IsFacingRight() != animations[i]->IsFacingRight()) {
			animations[i]->Flip();
		}
	}
}

void BoulderCreature::Move(float horizontal, float vertical) {
	if (can_take_input && hit_points > 0) {
		body->SetLinearVelocity(b2Vec2((horizontal / 100.0f) * speed * (running ? running_speed_multiplier : 1.0f), body->GetLinearVelocity().y));

		if (horizontal > 0) {
			SetFacingRight(true);
		} else if (horizontal < 0) {
			SetFacingRight(false);
		}
	}
}

void BoulderCreature::Jump() {
	if (can_take_input && hit_points > 0) {
		bool jumping = false;

		if (!IsInTheAir()) {
			jumping = true;
		}
		else if (has_double_jump) {
			jumping = true;
			has_double_jump = false;
		}

		if (jumping) {
			body->SetLinearVelocity(b2Vec2(body->GetLinearVelocity().x, -jump_power));
			SetInTheAir(true);
		}
	}
}

void BoulderCreature::Land() {
	landing_animation_timer->Start();
	SetInTheAir(false);
	has_double_jump = true;
	if (IsAnAttackActive()) {
		GetActiveAttack()->StopAttack();
	}
	if (jump_input_buffer->IsActive()) {
		Jump();
	}
	if (hit_points <= 0) {
		body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
	}
}

void BoulderCreature::Aggro(BoulderCreature* new_target) {
	if (is_hostile) {
		aggroCircleFixture->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
		deaggroCircleFixture->SetColor(0.0f, 1.0f, 0.0f, 1.0f);

		target = new_target;
	}
}

void BoulderCreature::Deaggro() {
	if (is_hostile) {
		aggroCircleFixture->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
		deaggroCircleFixture->SetColor(1.0f, 1.0f, 1.0f, 1.0f);

		target = nullptr;
	}
}

void BoulderCreature::SetInteractable(BoulderCreature* new_interactable) {
	if (interactable == nullptr || new_interactable == nullptr) {
		interactable = new_interactable;
	}
}

void BoulderCreature::UseAttack(int move_type) {
	if (can_take_input && hit_points > 0) {
		attacks[move_type]->InitiateAttack();

		if (move_type < (int)attacking_animations.size()) {
			attacking_animations[move_type]->Play();
		} else {
			attacking_animations[1]->Play();
		}
	}
}

void BoulderCreature::ReceiveHeal(int heal) {
	if (hit_points > 0) {
		cout << "got healed for " << heal << "!\n";
		hit_points += heal;

		if (hit_points > max_hit_points) {
			hit_points = max_hit_points;
		}
	}
}

void BoulderCreature::TakeDamage(int damage, sf::Vector2f knock_back, int hit_stun_frames) {
	if (hit_points > 0 && is_hittable) {
		cout << "got hit for " << damage << " damage and " << hit_stun_frames << " hit stun frames!\n";
		hit_stun_timer = new StatusTimer(hit_stun_frames);
		hit_stun_timer->Start();

		if (GettingHitSounds.size() > 0) {
			GettingHitSounds[rand() % (int)GettingHitSounds.size()]->play();
		}

		hit_points -= damage;

		if (hit_points <= 0) {
			hit_points = 0;
			cashinable_hit_point_value = 0;
			dying_animation_timer->Start();
			if ((int)activities.size() > 0) {
				for (int a = 0; a < (int)activities.size(); a++) {
					if (Contains(activities[a], "OnDeath")) {
						Singleton<SmashWorld>::Get()->ExecuteAction(activities[a]);
					}
				}
			}
			if (!IsInTheAir()) {
				body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
			}
		} else if (health_cash_in_timer != nullptr && !health_cash_in_timer->IsActive()) {
			body->SetLinearVelocity(b2Vec2(knock_back.x, knock_back.y));
			health_cash_in_timer->Start();
		}
	}
}

int BoulderCreature::TakeDamageWithLifeSteal(int damage, sf::Vector2f knock_back, int hit_stun_frames) {
	TakeDamage(damage, knock_back, hit_stun_frames);

	if (hit_points > 0 && cashinable_hit_point_value > 0) {
		int amount_of_life_stolen = cashinable_hit_point_value - hit_points;

		health_cash_in_timer->Stop();
		cashinable_hit_point_value = hit_points;

		return amount_of_life_stolen;
	}

	return 0;
}

Attack* BoulderCreature::GetActiveAttack() {
	Attack* active_attack = attacks[0];

	for (int i = 0; i < (int)attacks.size(); i++) {
		if (attacks[i]->IsAttacking()) {
			active_attack = attacks[i];
		}
	}

	return active_attack;
}

int BoulderCreature::GetActiveAttackIndex() {
	for (int i = 0; i < (int)attacks.size(); i++) {
		if (attacks[i]->IsAttacking()) {
			return i;
		}
	}

	return 0;
}

bool BoulderCreature::IsAnAttackActive() {
	for (int i = 0; i < (int)attacks.size(); i++) {
		if (attacks[i]->IsAttacking()) {
			return true;
		}
	}

	return false;
}

void BoulderCreature::StartTalking() {
	talking_animation_timer->Start();
}