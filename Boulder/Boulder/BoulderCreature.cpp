#pragma once

using namespace std;
#include "stdafx.h"
#include <iostream>
#include <sstream>
#include "BoulderCreature.h"
#include "Constants.h"
#include "SmashWorld.h"
#include "Utilities.h"
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
	attacksAreInterruptible = true;

	unit_type_json_data = jsonBestiariesData["DictOfUnits"][unit_type];

	cashinable_hit_point_value = hit_points = max_hit_points = unit_type_json_data["HitPoints"].asInt();
	interaction_radius = unit_type_json_data["InteractionRadius"].asFloat();
	can_take_input = false;

	cashinable_hit_point_drain_rate = 2;

	speed = unit_type_json_data["MovementSpeed"].asFloat();
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
	aggroCircleShape.m_radius = 5.0f;
	aggroCircleFixtureDef.shape = &aggroCircleShape;
	aggroCircleFixtureDef.isSensor = true;
	aggroCircleFixtureDef.m_color = new b2Color(1.0f, 1.0f, 1.0f, 1.0f);
	aggroCircleFixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->AGGRO_CIRCLE;
	aggroCircleFixtureDef.filter.maskBits = Singleton<SmashWorld>::Get()->PLAYER_CHARACTER;

	deaggroCircleShape.m_p.Set(0, 0); //position, relative to body position
	deaggroCircleShape.m_radius = 10.0f;
	deaggroCircleFixtureDef.shape = &deaggroCircleShape;
	deaggroCircleFixtureDef.isSensor = true;
	deaggroCircleFixtureDef.m_color = new b2Color(1.0f, 1.0f, 1.0f, 1.0f);
	deaggroCircleFixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->DEAGGRO_CIRCLE;
	deaggroCircleFixtureDef.filter.maskBits = Singleton<SmashWorld>::Get()->PLAYER_CHARACTER;

	interactionCircleShape.m_p.Set(0, 0); //position, relative to body position
	interactionCircleShape.m_radius = interaction_radius / 40.0f;
	interactionCircleFixtureDef.shape = &interactionCircleShape;
	interactionCircleFixtureDef.isSensor = true;
	interactionCircleFixtureDef.m_color = new b2Color(1.0f, 1.0f, 0.0f, 1.0f);
	interactionCircleFixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->INTERACTION_CIRCLE;
	interactionCircleFixtureDef.filter.maskBits = Singleton<SmashWorld>::Get()->PLAYER_CHARACTER;

	std::vector<float> vect_hurt_box;
	string strHurtBoxValues = unit_type_json_data["IdleAnimations"][0]["HurtBoxPerFrame"][0][0]["Box"].asString();
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

	hurt_box = new HurtBox(body, player_index, unit_type_json_data["IdleAnimations"][0]["HurtBoxPerFrame"][0][0]);

	body->SetFixedRotation(true);
	body->SetUserData(this);

	groundCheckShape.SetAsBox(vect_hurt_box[2] / 100.0f, 0.05f, b2Vec2(0.0f, (vect_hurt_box[3] / 100.0f) + (vect_hurt_box[2] / 160.0f)), 0.0f);
	groundCheckFixtureDef.shape = &groundCheckShape;
	groundCheckFixtureDef.isSensor = true;
	groundCheckFixtureDef.m_color = new b2Color(1.0f, 0.0f, 1.0f, 1.0f);

	groundCheckFixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->GROUND_CHECK;
	groundCheckFixtureDef.filter.maskBits = Singleton<SmashWorld>::Get()->PLATFORM;

	body->CreateFixture(&groundCheckFixtureDef);

	attacks.push_back(new Attack(body, player_index, Attack::FORWARD_SMASH, unit_type_json_data["AttackingAnimations"][0]));
	attacks.push_back(new Attack(body, player_index, Attack::DOWN_SMASH, unit_type_json_data["AttackingAnimations"][1]));

	sprite_scale = 1.0f;

	RightFootStepSoundFrameWalk = 0;
	LeftFootStepSoundFrameWalk = 0;
	RightFootStepSoundFrameRun = 0;
	LeftFootStepSoundFrameRun = 0;

	GettingHitSoundBuffers = std::vector<sf::SoundBuffer*>();
	GettingHitSounds = std::vector<sf::Sound*>();

	int getting_hit_sound_files_size = (int)unit_type_json_data["GettingHitSoundFiles"].size();
	for (int i = 0; i < getting_hit_sound_files_size; i++) {
		string soundFilePath = unit_type_json_data["GettingHitSoundFiles"][i].asString();
		size_t findResult = soundFilePath.find("Sound\\");
		string relativeFilePath = soundFilePath.substr(findResult);

		GettingHitSoundBuffers.push_back(new sf::SoundBuffer());
		GettingHitSoundBuffers[i]->loadFromFile(relativeFilePath);
		GettingHitSounds.push_back(new sf::Sound());
		GettingHitSounds[i]->setBuffer(*GettingHitSoundBuffers[i]);
	}

	LoadAllAnimations(unit_type, jsonBestiariesData);

	 attacks_size = (int)attacks.size();

	int projectile_active_animations_size = (int)projectile_active_animations.size();
	for (int i = 0; i < projectile_active_animations_size; i++) {
		projectiles.push_back(new BoulderProjectile(render_window, body, unit_type_json_data["ProjectileActiveAnimations"][i]["HitBoxPerFrame"][0][0], projectile_active_animations[i], projectile_hit_animations[i]));
	}
	if ((int)projectiles.size() > 0) {
		attackDistance = 3.5f;
	} else {
		attackDistance = 1.5f;
	}

	hit_stun_timer = new StatusTimer(1);
	health_cash_in_timer = new StatusTimer(360);
	jumpStartUpTimer = new StatusTimer(6);
	jumpInputBuffer = new StatusTimer(6); 
	jumpAfterWalkingOffLedgeBuffer = new StatusTimer(6);
	attack_input_buffer = new StatusTimer(6);
	attack_buffer_attack_index = 0;
	anAttackWasActiveLastFrame = false;

	jumpStartUpTimerWasActiveLastFrame = false;
	releasedJumpButton = true;

	int numberOfDyingAnimationFrames = dying_animations.size() > 0 ? dying_animations[0]->GetNumberOfFrames() : 0;
	dying_animation_timer = new StatusTimer(numberOfDyingAnimationFrames);

	int numberOfLandingAnimationFrames = landing_animations.size() > 0 ? landing_animations[0]->GetNumberOfFrames() : 1;
	landing_animation_timer = new StatusTimer(numberOfLandingAnimationFrames);

	talking_animation_timer = new StatusTimer(120);

	if (is_interactable) {
		float scale = 1.0f;

		interaction_button_texture = Singleton<AssetManager>().Get()->GetTexture("Images/XButton.png");
		interaction_button_sprite = new sf::Sprite(*interaction_button_texture);
		interaction_button_sprite->setScale(scale, scale);
	}

	DropThroughPassThroughPlatforms = false;
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
	cashinable_hit_point_value = hit_points;
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
	projectile_active_animations = LoadAnimations("ProjectileActiveAnimations", unit_type, jsonBestiariesData);
	projectile_hit_animations = LoadAnimations("ProjectileHitAnimations", unit_type, jsonBestiariesData);
}

std::vector<SpriteAnimation*> BoulderCreature::LoadAnimations(string animations_name, string unit_type, Json::Value jsonData) {
	std::vector<SpriteAnimation*> animations = std::vector<SpriteAnimation*>();
	Json::Value unit_type_json_bestiary_data = jsonData["DictOfUnits"][unit_type];

	int number_of_animations = (int)unit_type_json_bestiary_data[animations_name].size();
	for (int i = 0; i < number_of_animations; i++)
	{
		string filePath = unit_type_json_bestiary_data[animations_name][i]["FilePath"].asString();

		if (filePath != "") {
			size_t findResult = filePath.find("Units\\");
			string relativeFilePath = filePath.substr(findResult);

			animations.push_back(new SpriteAnimation(render_window,
				relativeFilePath,
				unit_type_json_bestiary_data[animations_name][i]["FrameDimensionsX"].asInt(),
				unit_type_json_bestiary_data[animations_name][i]["FrameDimensionsY"].asInt(),
				unit_type_json_bestiary_data[animations_name][i]["NumberOfFrames"].asInt(),
				unit_type_json_bestiary_data[animations_name][i]["FramesPerColumn"].asInt(),
				unit_type_json_bestiary_data[animations_name][i]["FramesPerRow"].asInt(),
				sprite_scale, sf::Color::White, animations_name != "JumpingAnimations" && animations_name != "JumpApexAnimations"));
		}

		int projectile_spawn_box_per_frame_size = unit_type_json_bestiary_data[animations_name][i]["ProjectileSpawnBoxPerFrame"].size();
		for (int f = 0; f < projectile_spawn_box_per_frame_size; f++) {
			if (unit_type_json_bestiary_data[animations_name][i]["ProjectileSpawnBoxPerFrame"][f][0]["Name"].asString() != "") {
				std::vector<float> usable_box_data = Utilities::StringBoxDataToUsableVectorBoxData(unit_type_json_bestiary_data[animations_name][i]["ProjectileSpawnBoxPerFrame"][f][0]["Box"].asString());

				ProjectileFiringFrame projectile_firing_frame;
				projectile_firing_frame.attack_animation_index = animations.size() - 1;
				projectile_firing_frame.firing_frame = f;
				projectile_firing_frame.relative_spawning_position_x = usable_box_data[0] + (usable_box_data[2] / 2.0f);
				projectile_firing_frame.relative_spawning_position_y = usable_box_data[1] + (usable_box_data[3] / 2.0f);

				projectile_firing_frames.push_back(projectile_firing_frame);

				break;
			}
		}

		int attack_sound_file_per_frame_size = unit_type_json_bestiary_data[animations_name][i]["AttackSoundFilePerFrame"].size();
		for (int f = 0; f < attack_sound_file_per_frame_size; f++) {
			if (unit_type_json_bestiary_data[animations_name][i]["AttackSoundFilePerFrame"][f].asString() != "") {
				string soundFilePath = unit_type_json_bestiary_data[animations_name][i]["AttackSoundFilePerFrame"][f].asString();
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

		int right_foot_sound_file_per_frame_size = unit_type_json_bestiary_data[animations_name][i]["RightFootSoundFilePerFrame"].size();
		for (int f = 0; f < right_foot_sound_file_per_frame_size; f++) {
			if (unit_type_json_bestiary_data[animations_name][i]["RightFootSoundFilePerFrame"][f].asString() != "") {
				string soundFilePath = unit_type_json_bestiary_data[animations_name][i]["RightFootSoundFilePerFrame"][f].asString();
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

		int left_foot_sound_file_per_frame_size = unit_type_json_bestiary_data[animations_name][i]["LeftFootSoundFilePerFrame"].size();
		for (int f = 0; f < left_foot_sound_file_per_frame_size; f++) {
			if (unit_type_json_bestiary_data[animations_name][i]["LeftFootSoundFilePerFrame"][f].asString() != "") {
				string soundFilePath = unit_type_json_bestiary_data[animations_name][i]["LeftFootSoundFilePerFrame"][f].asString();
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

	return animations;
}

bool BoulderCreature::IfShouldUpdate(sf::Vector2f player_screen_pos, sf::Vector2f viewport_dimensions) {
	sf::Vector2f body_screen_pos = GetScreenPosition();

	if (abs(player_screen_pos.x - body_screen_pos.x) > viewport_dimensions.x * 1.25f) {
		return false;
	}
	
	if (abs(player_screen_pos.y - body_screen_pos.y) > viewport_dimensions.y * 1.25f) {
		return false;
	}

	return true;
}

void BoulderCreature::Update(sf::Int64 curr_frame, sf::Int64 delta_time) {
	Creature::Update(curr_frame, delta_time);

	b2Vec2 body_lin_vel = body->GetLinearVelocity();

	if ((int)platformContacts.size() > 0) {
		b2Vec2 platform_lin_vel = platformContacts[0]->GetBody()->GetLinearVelocity();

		body->SetLinearVelocity(b2Vec2(body_lin_vel.x + platform_lin_vel.x, body_lin_vel.y));
	}

	if (IsInTheAir() && body_lin_vel.y > maxVerticalVelocityReached) {
		maxVerticalVelocityReached = body_lin_vel.y;
	}

	can_take_input = !hit_stun_timer->IsActive();
	//if (can_take_input) {
	//	can_take_input = !IsAnAttackActive();
	//}

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
	
	for (int i = 0; i < attacks_size; i++) {
		attacks[i]->Update(curr_frame, IsFacingRight());
	}

	if (hit_stun_timer->IsActive()) {
		topCircleFixture->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
		botCircleFixture->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
		centerBoxFixture->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
	} else {
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
	} else if (State == STATE_ATTACKING) {
		int projectile_firing_frames_size = (int)projectile_firing_frames.size();
		for (int i = 0; i < projectile_firing_frames_size; i++) {
			if (projectile_firing_frames[i].attack_animation_index == GetActiveAttackIndex()) {
				for (int p = 0; p < (int)projectiles.size(); p++) {
					if (attacking_animations[GetActiveAttackIndex()]->GetCurrentFrame() == projectile_firing_frames[i].firing_frame && !projectiles[p]->IsActive()) {
						projectiles[p]->Activate(body->GetPosition().x + (IsFacingRight() ? projectile_firing_frames[i].relative_spawning_position_x : -projectile_firing_frames[i].relative_spawning_position_x), body->GetPosition().y + projectile_firing_frames[i].relative_spawning_position_y, IsFacingRight());
					}
				}
			}
		}
	}

	int projectiles_size = (int)projectiles.size();
	for (int i = 0; i < projectiles_size; i++) {
		projectiles[i]->Update();// curr_frame, delta_time);
	}
}

void BoulderCreature::UpdateBehavior() {
	if (target != nullptr)
	{
		float delta_x = target->GetBody()->GetPosition().x - body->GetPosition().x;
		float delta_y = target->GetBody()->GetPosition().y - body->GetPosition().y;

		float distance = sqrtf(powf(delta_x, 2) + powf(delta_y, 2));

		if (distance < attackDistance) {
			if (!IsAnAttackActive()) {
				if (target->GetBody()->GetPosition().x < body->GetPosition().x && IsFacingRight()) {
					SetFacingRight(false);
				}
				else if (target->GetBody()->GetPosition().x > body->GetPosition().x && !IsFacingRight()) {
					SetFacingRight(true);
				}

				UseAttack(0);
			}
		} else {
			if (target->GetBody()->GetPosition().x < body->GetPosition().x) {
				movement = -100.0f;
				running = true;
			} else if (target->GetBody()->GetPosition().x > body->GetPosition().x) {
				movement = 100.0f;
				running = true;
			}
		}
	} else if (current_frame % 60 == 0) {
		int rng = rand() % 5;

		if (rng == 0) {
			movement = -100.0f;
			running = false;
		} else if (rng == 1) {
			movement = 0.0f;
			running = false;
		} else if (rng == 2) {
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
	} else {
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
		else if (movement != 0) {
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

	int projectiles_size = (int)projectiles.size();
	for (int i = 0; i < projectiles_size; i++) {
		projectiles[i]->Draw(camera_position);
	}

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

	std::size_t foundPlayer = name.find("Player");
	std::size_t foundBoss = name.find("Boss");
	if (foundPlayer == std::string::npos && foundBoss == std::string::npos) {
		float percent_health = ((float)hit_points / (float)max_hit_points);

		healthBarRect->setSize(sf::Vector2f(starting_health_bar_width * percent_health, 5.0f));
		healthBarRect->setPosition(sf::Vector2f((body->GetPosition().x - 0.5f - camera_position.x) * 40.0f, (body->GetPosition().y - 0.6f - camera_position.y) * 40.0f));
		healthBarRect->setFillColor(sf::Color((int)(0.0f + (255.0f - (float)hit_points / (float)max_hit_points) * 255.0f), (int)((float)hit_points / (float)max_hit_points * 255.0f), 0, 255));

		float percent_cash_in_health = ((float)cashinable_hit_point_value / (float)max_hit_points);

		cashInHealthBarRect->setSize(sf::Vector2f(starting_health_bar_width * percent_cash_in_health, 5.0f));
		cashInHealthBarRect->setPosition(sf::Vector2f((body->GetPosition().x - 0.5f - camera_position.x) * 40.0f, (body->GetPosition().y - 0.6f - camera_position.y) * 40.0f));

		render_window->draw(*cashInHealthBarRect);
	}

	if (hit_points > 0) {
		render_window->draw(*healthBarRect);
	}

	if (is_interactable && draw_interaction_button) {
		current_y_offset += (int)((current_frame % 60) - 30) / 120.0f;

		interaction_button_sprite->setPosition(sf::Vector2f((body->GetPosition().x - camera_position.x) * 40.0f - 20.0f, (body->GetPosition().y - camera_position.y) * 40.0f + current_y_offset - 10.0f));
		render_window->draw(*interaction_button_sprite);
	}
}

void BoulderCreature::AddActivaty(string activity) {
	std::vector<string> vstrings = Utilities::Split(activity, ';');
	int vstrings_size = (int)vstrings.size();

	for (int i = 0; i < vstrings_size; i++) {
		if (vstrings[i] != "") {
			if (std::find(activities.begin(), activities.end(), vstrings[i]) == activities.end()) {
				activities.push_back(vstrings[i]);
			}
		}
	}
}

void BoulderCreature::FlipAnimationsIfNecessary(std::vector<SpriteAnimation*> animations) {
	int animations_size = (int)animations.size();
	for (int i = 0; i < animations_size; i++) {
		if (IsFacingRight() != animations[i]->IsFacingRight()) {
			animations[i]->Flip();
		}
	}
}

void BoulderCreature::Move(float horizontal, float vertical) {
	if (can_take_input && hit_points > 0 && !landing_animation_timer->IsActive()) {
		if (!IsInTheAir()) {
			if (!IsAnAttackActive()) {
				b2Vec2 lin_vel = body->GetLinearVelocity();
				float hyp = (horizontal / 100.0f) * speed * (running ? running_speed_multiplier : 1.0f);

				body->SetLinearVelocity(b2Vec2(hyp, lin_vel.y));// 0.0f));

				//if ((lin_vel.x == 0.0f || lin_vel.y == 0.0f) && hyp > 0.1f) {
				//	body->SetLinearVelocity(b2Vec2(hyp, 0.0f));
				////} else if (lin_vel.x == 0.0f) {
				////	body->SetLinearVelocity(b2Vec2(hyp, body->GetLinearVelocity().y));
				////} else if (lin_vel.y == 0.0f) {
				////	body->SetLinearVelocity(b2Vec2(hyp, body->GetLinearVelocity().y));
				//} else {
				//	if (hyp != 0.0f) {
				//		float32 radians = 0.0f;
				//		float32 angle = 0.0f;
				//
				//		radians = atan(lin_vel.y / lin_vel.x);
				//		angle = radians * 180.0f / 3.141592653589793238463f;
				//
				//		float32 hor = hyp * cosf(radians);
				//		float32 ver = hyp * sinf(radians);
				//
				//		//if ((lin_vel.x > 0.0f && hor < 0.0f) || (lin_vel.x < 0.0f && hor > 0.0f)) {
				//		//	hor *= -1.0f;
				//		//}
				//		//if ((lin_vel.y < 0.0f && ver < 0.0f) || (lin_vel.y > 0.0f && ver > 0.0f)) {
				//		//	ver *= -1.0f;
				//		//}
				//
				//		body->SetLinearVelocity(b2Vec2(hor, ver));
				//
				//		if (Utilities::Contains(name, "Player")) {
				//			float32 len = body->GetLinearVelocity().Length();
				//			cout << len << "\t" << hyp << "\n";
				//		}
				//	}
				//}

				if (horizontal > 0) {
					SetFacingRight(true);
				} else if (horizontal < 0) {
					SetFacingRight(false);
				}
			}
		} else {
			float old_body_vel_x = body->GetLinearVelocity().x;
			float x_drift = horizontal / 200.0f;
			float x_vel = old_body_vel_x + x_drift;
			float max_speed = 0.0f;

			if (maxAirSpeed > 0 && maxAirSpeed > speed) {
				max_speed = maxAirSpeed;

				if (x_vel < maxAirSpeed) {
					maxAirSpeed = x_vel;
				}
			} else if (maxAirSpeed < 0 && maxAirSpeed < -speed) {
				max_speed = maxAirSpeed;

				if (x_vel > maxAirSpeed) {
					maxAirSpeed = x_vel;
				}
			} else {
				max_speed = x_vel < 0 ? -speed : speed;
			}

			if ((max_speed > 0 && x_vel > max_speed) || (max_speed < 0 && x_vel < max_speed)) {
				x_vel = max_speed;
			}

			//if (max_speed > 0 && x_vel > max_speed) {
			//	x_vel = max_speed;
			//} else if (max_speed < 0 && x_vel < max_speed) {
			//	x_vel = max_speed;
			//}

			body->SetLinearVelocity(b2Vec2(x_vel, body->GetLinearVelocity().y));
		}
	}
}

bool BoulderCreature::IsJumping() {
	return jumpStartUpTimer->IsActive();
}

void BoulderCreature::StartJump() {
	if (can_take_input && hit_points > 0 && !IsAnAttackActive() && !landing_animation_timer->IsActive()) {
		jumpStartUpTimer->Start();
	}
}

void BoulderCreature::ActuallyJump(bool short_hop) {
	if (can_take_input && hit_points > 0 && !IsAnAttackActive() && !landing_animation_timer->IsActive()) {
		bool jumping = false;

		if (!IsInTheAir()) {
			jumping = true;
		} else if (has_double_jump) {
			jumping = true;
			has_double_jump = false;
		}

		if (jumping) {
			maxAirSpeed = body->GetLinearVelocity().x;
			body->SetLinearVelocity(b2Vec2(maxAirSpeed, short_hop ? -jump_power * 0.8f : -jump_power));
			SetInTheAir(true);
		}
	}
}

void BoulderCreature::ReverseHorizontalDirectionIfInHitStun() {
	if (hit_stun_timer->IsActive() && IsInTheAir()) {
		b2Vec2 linear_velocity = body->GetLinearVelocity();
		body->SetLinearVelocity(b2Vec2(-linear_velocity.x, linear_velocity.y));
	}
}

void BoulderCreature::AddPlatformContact(Box2DRigidBody* platform) {
	if ((int)platformContacts.size() == 0) {
		Land();
	}

	platformContacts.push_back(platform);
}

void BoulderCreature::RemovePlatformContact(Box2DRigidBody* platform) {
	if ((int)platformContacts.size() == 1) {
		jumpAfterWalkingOffLedgeBuffer->Start();
		maxAirSpeed = body->GetLinearVelocity().x;
		SetInTheAir(true);
	}

	std::vector<Box2DRigidBody*>::iterator found_item = std::find(platformContacts.begin(), platformContacts.end(), platform);
	if (found_item != platformContacts.end())
	{
		platformContacts.erase(found_item);
	}
}

void BoulderCreature::Land() {
	if (maxVerticalVelocityReached > 20.0f) {
		//landing_animation_timer->Start();
	}
	maxVerticalVelocityReached = 0.0f;
	SetInTheAir(false);
	has_double_jump = true;
	fastFalling = false;
	if (IsAnAttackActive()) {
		Attack* active_attack = GetActiveAttack();

		if (active_attack != nullptr) {
			active_attack->StopAttack();
		}
	}
	if (jumpInputBuffer->IsActive()) {
		StartJump();
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
	if (interactable == nullptr) {
		interactable = new_interactable;
		if (interactable != nullptr) {
			interactable->draw_interaction_button = true;
		}
	} else if (new_interactable == nullptr) {
		if (interactable != nullptr) {
			interactable->draw_interaction_button = false;
		}
		interactable = new_interactable;
	}
}

void BoulderCreature::UseAttack(int move_type, bool activate_buffer) {
	if (activate_buffer) {
		attack_input_buffer->Start();
		attack_buffer_attack_index = move_type;
	}

	if (IsInTheAir()) {
		if (move_type == Attack::UP_SMASH || move_type == Attack::FORWARD_SMASH || move_type == Attack::DOWN_SMASH || move_type == Attack::JAB) {
			return;
		}
	} else {
		if (move_type == Attack::UP_AIR || move_type == Attack::FORWARD_AIR || move_type == Attack::DOWN_AIR || move_type == Attack::BACK_AIR || move_type == Attack::NEUTRAL_AIR) {
			return;
		}
	}

	if (can_take_input && hit_points > 0 && !landing_animation_timer->IsActive() && !hit_stun_timer->IsActive() && !IsAnAttackActive()) {
		attacks[move_type]->InitiateAttack();

		damageTakenSinceAttackStarted = 0;

		if (move_type < (int)attacking_animations.size()) {
			attacking_animations[move_type]->Play();
		} else {
			attacking_animations[1]->Play();
		}
	}
}

void BoulderCreature::ReceiveHeal(int heal) {
	if (hit_points > 0) {
		hit_points += heal;

		if (hit_points > max_hit_points) {
			hit_points = max_hit_points;
		}
	}
}

void BoulderCreature::TakeDamage(int damage, sf::Vector2f knock_back, int hit_stun_frames, bool pop_up_grounded_enemies) {
	if (is_hittable) {
		if (GettingHitSounds.size() > 0) {
			GettingHitSounds[rand() % (int)GettingHitSounds.size()]->play();
		}
	}

	if (hit_points > 0 && is_hittable) {
		Singleton<SmashWorld>::Get()->ScreenShake((int)(hit_stun_frames > 0 ? hit_stun_frames : -hit_stun_frames) / 8.0f);

		if (hit_stun_timer->IsActive()) {
			knockBackMultiplier += knockBackMultiplierIncreasePerHit;
			hitStunMultiplier -= hitStunMultiplierDecreasePerHit;

			if (hitStunMultiplier < minimumHitStunMultiplier) {
				hitStunMultiplier = minimumHitStunMultiplier;
			}
		} else {
			knockBackMultiplier = 1.0f;
			hitStunMultiplier = 1.0f;
		}

		damageTakenSinceAttackStarted += damage;

		Attack* active_attack = GetActiveAttack();
		bool interrupt_attack = active_attack == nullptr || (active_attack != nullptr && (damageTakenSinceAttackStarted > active_attack->Poise || active_attack->IsInRecoveryFrames()));

		if (hit_points <= 0) {
			knockBackMultiplier = 1.0f;
		}

		if (interrupt_attack) {
			hit_stun_timer = new StatusTimer((int)((hit_stun_frames > 0 ? hit_stun_frames : -hit_stun_frames) * hitStunMultiplier));
			hit_stun_timer->Start();

			if (active_attack != nullptr) {
				active_attack->StopAttack();
			}

			if (pop_up_grounded_enemies && !IsInTheAir()) {
				body->SetLinearVelocity(b2Vec2(knock_back.x * knockBackMultiplier, -knock_back.y * knockBackMultiplier));
			} else {
				body->SetLinearVelocity(b2Vec2(knock_back.x * knockBackMultiplier, knock_back.y * knockBackMultiplier));
			}
		}

		if (hit_points > 0 && hit_points - damage <= 0) {
			hit_points = 0;
			cashinable_hit_point_value = 0;
			dying_animation_timer->Start();

			Attack* active_attack = GetActiveAttack();
			if (active_attack != nullptr) {
				active_attack->StopAttack();
			}

			int activities_size = (int)activities.size();
			for (int a = 0; a < activities_size; a++) {
				if (Contains(activities[a], "OnDeath")) {
					Singleton<SmashWorld>::Get()->ExecuteAction(activities[a]);
				}
			}

			if (player_index != 0) {
				Singleton<SmashWorld>::Get()->EnemyDied(50);
			}
		} else {
			hit_points -= damage;

			if (health_cash_in_timer != nullptr && !health_cash_in_timer->IsActive()) {
				health_cash_in_timer->Start();
			}
		}
	} else {
		Singleton<SmashWorld>::Get()->ScreenShake(1.5f);
	}
}

int BoulderCreature::TakeDamageWithLifeSteal(int damage, sf::Vector2f knock_back, int hit_stun_frames, bool pop_up_grounded_enemies) {
	TakeDamage(damage, knock_back, hit_stun_frames, pop_up_grounded_enemies);

	if (hit_points > 0 && cashinable_hit_point_value > 0) {
		int amount_of_life_stolen = cashinable_hit_point_value - hit_points;

		health_cash_in_timer->Stop();
		cashinable_hit_point_value = hit_points;

		return amount_of_life_stolen;
	}

	return 0;
}

Attack* BoulderCreature::GetActiveAttack() {
	for (int i = 0; i < attacks_size; i++) {
		if (attacks[i]->IsAttacking()) {
			return attacks[i];
		}
	}

	return nullptr;
}

int BoulderCreature::GetActiveAttackIndex() {
	for (int i = 0; i < attacks_size; i++) {
		if (attacks[i]->IsAttacking()) {
			return i;
		}
	}

	return 0;
}

bool BoulderCreature::IsAnAttackActive() {
	for (int i = 0; i < attacks_size; i++) {
		if (attacks[i]->IsAttacking()) {
			return true;
		}
	}

	return false;
}

void BoulderCreature::StartTalking() {
	talking_animation_timer->Start();
}

int BoulderCreature::GetDamageOfCurrentAttack() {
	Attack* active_attack = GetActiveAttack();

	if (active_attack != nullptr) {
		return active_attack->GetDamage();
	}

	return 0;
}

void BoulderCreature::UpdateEffectsVolumes(float new_effects_volume) {
	RightFootStepSound.setVolume(new_effects_volume);
	LeftFootStepSound.setVolume(new_effects_volume);

	for (int i = 0; i < (int)GettingHitSounds.size(); i++) {
		GettingHitSounds[i]->setVolume(new_effects_volume);
	}
}

void BoulderCreature::AddAnger(int anger_amount) {
}

bool BoulderCreature::IsInHitStun() {
	return hit_stun_timer->IsActive();
}